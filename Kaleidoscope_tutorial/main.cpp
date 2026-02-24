//
// Created by zuxinggu on 2025/8/4.
//

#include "ast.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"

#include <iostream>
#include <string>
#include <unordered_map>

static std::string Input;
static int InputIndex = 0;

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

/// return tokens [0-255] to represent unknown character, otherwise known tokens
enum Token {
    tok_eof = -1,

    // key word
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,

    // if-expr
    tok_if = -6,
    tok_then = -7,
    tok_else = -8,

    // for-expr
    tok_for = -9,
    tok_in = -10,

    // 自定义操作符号
    tok_binary = -11,
    tok_unary = -12,

    // var
    tok_var = -13,
};

static std::string Id; // current id if tok_identifier
static double Number; // current num value if tok_number

/// lexer
/// gettok _ return the next token from standard input.
static int gettok() {
    static int LastChar = ' ';
    // skip any space
    while (isspace(LastChar)) {
        LastChar = Input[InputIndex++];
    }
    // check id
    if (isalpha(LastChar)) {
        Id = LastChar;
        while (isalnum(LastChar = Input[InputIndex++])) {
            Id += LastChar;
        }
        // special id, def, 这里处理keyword
        if (Id == "def") {
            return tok_def;
        }
        if (Id == "extern") {
            return tok_extern;
        }
        if (Id == "if") {
            return tok_if;
        }
        if (Id == "then") {
            return tok_then;
        }
        if (Id == "else") {
            return tok_else;
        }
        if (Id == "for")
            return tok_for;
        if (Id == "in")
            return tok_in;
        if (Id == "binary")
            return tok_binary;
        if (Id == "unary")
            return tok_unary;
        if (Id == "var")
            return tok_var;
        return tok_identifier;
    }
    // check number
    if (isdigit(LastChar) || LastChar == '.') {
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = Input[InputIndex++];
        } while (isdigit(LastChar) || LastChar == '.');
        Number = std::stod(NumStr);
        return tok_number;
    }
    // comment
    if (LastChar == '#') {
        do {
            LastChar = Input[InputIndex++];
        } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
        if (LastChar != EOF) {
            // continue to next token
            return gettok();
        }
    }
    if (LastChar == EOF) {
        return tok_eof;
    }
    int ThisChar = LastChar;
    LastChar = Input[InputIndex++]; // always move LastChar to the next one
    return ThisChar;
}
//===----------------------------------------------------------------------===//
// Lexer end
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//
/// 当前的token是什么，一个缓冲
static int CurTok;
/// 获取下一个Token是什么，用来触发递归向下的parse
static int getNextToken() {
    CurTok = gettok();
    return CurTok;
}
/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}

/// --- 从这里开始，我们根据spec来从小结构到大结构来实现解析过程
/// 我们对于解析，每次都是先拿下一个token，然后根据下一个token的情况来调用parse函数

/// A. 解析数字 - numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr() {
    // 根据当前情况，知道是数字，直接拿
    auto Result = std::make_unique<NumberExprAST>(Number);
    getNextToken(); // 跳过符号
    return std::move(Result);
}

static std::unique_ptr<ExprAST> ParseExpression();

/// B. 解析括号表达式 - parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); // 跳过当前的'('
    auto V = ParseExpression();
    if (!V) {
        return nullptr; // 出错了
    }
    if (CurTok != ')') // 此时判断是不是')'
        return LogError("expected ')'");
    getNextToken(); // 跳过当前的')'
    return std::move(V);
}

/// C. 解析标识符表达式： identifierexpr ::= identifier | identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = Id;
    getNextToken(); // 跳过当前ID，看后面
    if (CurTok != '(') {
        // 是一个VariableExprAST
        return std::make_unique<VariableExprAST>(IdName);
    }
    // 是(,那么就是一个call
    getNextToken(); // 跳过（
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        while (true) {
            if (auto Arg = ParseExpression()) {
                Args.push_back(std::move(Arg));
            } else {
                return nullptr;
            }
            if (CurTok == ')') {
                break;
            }
            if (CurTok != ',') {
                return LogError("expected ','");
            }
            getNextToken(); // 下一个参数
        }
    }
    getNextToken(); // 跳过）
    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

/// 处理if表达式
static std::unique_ptr<ExprAST> ParseIfExpr() {
    getNextToken(); // 跳过if
    auto Cond = ParseExpression();
    if (!Cond) return nullptr;
    if (CurTok != tok_then)
        return LogError("expected 'then'");
    getNextToken(); // 跳过then
    auto Then = ParseExpression();
    if (!Then)
        return nullptr;

    if (CurTok != tok_else)
        return LogError("expected else");

    getNextToken();

    auto Else = ParseExpression();
    if (!Else)
        return nullptr;

    return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then),
                                        std::move(Else));
}

/// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
static std::unique_ptr<ExprAST> ParseForExpr() {
    getNextToken();  // eat the for.

    if (CurTok != tok_identifier)
        return LogError("expected identifier after for");

    std::string IdName = Id;
    getNextToken();  // eat identifier.

    if (CurTok != '=')
        return LogError("expected '=' after for");
    getNextToken();  // eat '='.

    auto Start = ParseExpression();
    if (!Start)
        return nullptr;
    if (CurTok != ',')
        return LogError("expected ',' after for start value");
    getNextToken();

    auto End = ParseExpression();
    if (!End)
        return nullptr;

    // The step value is optional.
    std::unique_ptr<ExprAST> Step;
    if (CurTok == ',') {
        getNextToken();
        Step = ParseExpression();
        if (!Step)
            return nullptr;
    }

    if (CurTok != tok_in)
        return LogError("expected 'in' after for");
    getNextToken();  // eat 'in'.

    auto Body = ParseExpression();
    if (!Body)
        return nullptr;

    return std::make_unique<ForExprAST>(IdName, std::move(Start),
                                         std::move(End), std::move(Step),
                                         std::move(Body));
}

/// varexpr ::= 'var' identifier ('=' expression)?
//                    (',' identifier ('=' expression)?)* 'in' expression
static std::unique_ptr<ExprAST> ParseVarExpr() {
    getNextToken(); // eat the var.

    std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;

    // At least one variable name is required.
    if (CurTok != tok_identifier)
        return LogError("expected identifier after var");

    while (true) {
        std::string Name = Id;
        getNextToken(); // eat identifier.

        // Read the optional initializer.
        std::unique_ptr<ExprAST> Init = nullptr;
        if (CurTok == '=') {
            getNextToken(); // eat the '='.

            Init = ParseExpression();
            if (!Init)
                return nullptr;
        }

        VarNames.emplace_back(std::make_pair(Name, std::move(Init)));

        // End of var list, exit loop.
        if (CurTok != ',')
            break;
        getNextToken(); // eat the ','.

        if (CurTok != tok_identifier)
            return LogError("expected identifier list after var");
    }

    // At this point, we have to have 'in'.
    if (CurTok != tok_in)
        return LogError("expected 'in' keyword after 'var'");
    getNextToken(); // eat 'in'.

    auto Body = ParseExpression();
    if (!Body)
        return nullptr;

    return std::make_unique<VarExprAST>(std::move(VarNames), std::move(Body));
}

/// D: 解析一个基础表达式
/// primary
///   ::= identifierexpr C
///   ::= numberexpr A
///   ::= parenexpr B
static std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
        default:
            return LogError("unknown token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
        case tok_if:
            return ParseIfExpr();
        case tok_for:
            return ParseForExpr();
        case tok_var:
            return ParseVarExpr();
    }
}

/// unary
///   ::= primary
///   ::= '!' unary
static std::unique_ptr<ExprAST> ParseUnary() {
    // If the current token is not an operator, it must be a primary expr.
    if (!isascii(CurTok) || CurTok == '(' || CurTok == ',')
        return ParsePrimary();

    // If this is a unary operator, read it.
    int Opc = CurTok;
    getNextToken();
    if (auto Operand = ParseUnary())
        return std::make_unique<UnaryExprAST>(Opc, std::move(Operand));
    return nullptr;
}

/// E: 解析二元表达式，这个会复杂一些，因为涉及到优先级
/// 所以，我们先注册符号的优先级，在解析的过程中，一定是形如 Lhs op Rhs [op2 T]
/// （1）如果后面的符号op2优先级高，那么就以当前的Rhs，作为下一个的Lhs进行，结果一起作为结果
/// 也就是：Lhs op (Lhs=Rhs, op2, Rhs=T)
/// (2) 反之op2低，那么就先合并Lhs和Rhs，一起作为下一个的Lhs，(Lhs=Lhs, op, Rhs=Rhs) op2 T
static std::unordered_map<char, int> BinopPrecedence = {
    {'=', 2},
    {'<', 10},
    {'+', 20},
    {'-', 20},
    {'*', 40},
};
static int GetTokPrecedence() {
    if (!isascii(CurTok)) {
        return -1;
    }
    int TokPrecedence = BinopPrecedence[CurTok];
    if (TokPrecedence <= 0) {
        return -1;
    }
    return TokPrecedence;
}
/// 二元表达式，我们只关注右侧; 也就是说，我已经有一个CurLHS了，我要开始看它的右边怎么处理
static std::unique_ptr<ExprAST> ParseBinOpRHS(int CurExprPrec, std::unique_ptr<ExprAST> CurLHS) {
    while (true) {
        int TokPrecedence = GetTokPrecedence(); // 拿到下一个符号的优先级
        if (TokPrecedence < CurExprPrec) { // 下一个低，那么可以了，不用管后面了；这个CurLHS要和之前的结合
            std::cout << "[debug: ] ParseBinOpRHS, " << CurLHS->printToStr() << std::endl;
            return CurLHS;
        }

        int BinOp = CurTok;
        getNextToken(); // 跳过下一个符号
        auto RHS = ParseUnary();
        if (!RHS) return nullptr;
        std::cout << "[debug: ] ParseBinOpRHS, " << RHS->printToStr() << std::endl;
        // 现在有了lhs，rhs，那么要看再下一个了这决定了RHS怎么处理
        int NextPrec = GetTokPrecedence();
        if (TokPrecedence < NextPrec) {
            // 下一个更高，那么RHS要和后面的结合
            RHS = ParseBinOpRHS(TokPrecedence + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
            std::cout << "[debug: ] ParseBinOpRHS, " << RHS->printToStr() << std::endl;
        }
        // RHS都处理好了，应该合并的都合并了，那么就处理CurExprPrec这个
        CurLHS = std::make_unique<BinaryExprAST>(BinOp, std::move(CurLHS), std::move(RHS));
        std::cout << "[debug: ] ParseBinOpRHS, " << CurLHS->printToStr() << std::endl;
    }
}


/// F: 解析表达式
/// expression
///   ::= primary binoprhs
///
static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParseUnary();
    if (!LHS)
        return nullptr;
    return ParseBinOpRHS(0, std::move(LHS));
}

/// G: 函数声明
// prototype ::= id '(' id* ')' // 函数签名
//           ::= binary LETTER number? (id, id)
//           ::= unary LETTER number? (id)
static std::unique_ptr<PrototypeAST> ParsePrototype() {
    std::string FnName;

    unsigned Kind = 0;  // 0 = identifier, 1 = unary, 2 = binary.
    unsigned BinaryPrecedence = 30; // 默认的优先级

    switch (CurTok) {
        default:
            return LogErrorP("Expected function name in prototype");
        case tok_identifier:
            FnName = Id;
            Kind = 0;
            getNextToken(); // 跳过当前Id,取下一个
            break;
        case tok_unary:
            getNextToken();
            if (!isascii(CurTok))
                return LogErrorP("Expected unary operator");
            FnName = "unary";
            FnName += (char)CurTok;
            Kind = 1;
            getNextToken(); // 函数名就是unary+op
            break;
        case tok_binary:
            getNextToken();
            if (!isascii(CurTok))
                return LogErrorP("Expected binary operator");
            FnName = "binary";
            FnName += (char)CurTok;
            Kind = 2;
            getNextToken();

            // Read the precedence if present.
            if (CurTok == tok_number) {
                if (Number < 1 || Number > 100)
                    return LogErrorP("Invalid precedence: must be 1..100");
                BinaryPrecedence = (unsigned)Number;
                getNextToken();
            }
            break;
    }

    if (CurTok != '(')
        return LogErrorP("Expected '(' in prototype");

    std::vector<std::string> ArgNames;
    while (getNextToken() == tok_identifier)
        ArgNames.push_back(Id);
    if (CurTok != ')')
        return LogErrorP("Expected ')' in prototype");

    // success.
    getNextToken();  // eat ')'.

    // Verify right number of names for operator.
    if (Kind && ArgNames.size() != Kind)
        return LogErrorP("Invalid number of operands for operator");

    return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames), Kind != 0,
                                           BinaryPrecedence);
}
/// H: 函数定义
/// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {
    getNextToken(); // eat def.
    auto Proto = ParsePrototype();
    if (!Proto)
        return nullptr;

    if (auto E = ParseExpression())
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    return nullptr;
}

/// I：外部声明
/// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {
    getNextToken(); // eat extern.
    return ParsePrototype();
}

/// J: 顶层表达式，只有函数体
/// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        // Make an anonymous proto.
        auto Proto = std::make_unique<PrototypeAST>("__anon_expr",
                                                    std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}
//===----------------------------------------------------------------------===//
// Parser end
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

static std::unique_ptr<llvm::LLVMContext> TheContext; // 上下文信息，数据结构
static std::unique_ptr<llvm::Module> TheModule; // 组织函数和全局变量
static std::unique_ptr<llvm::IRBuilder<>> Builder; // 组织IR指令
static std::map<std::string, llvm::AllocaInst *> NamedValues; // 记录当前符号表，这样所有的变量都可以用内存地址来控制
static std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos; // 记录所有的函数定义

/// 优化
static std::unique_ptr<llvm::FunctionPassManager> TheFPM;
static std::unique_ptr<llvm::LoopAnalysisManager> TheLAM;
static std::unique_ptr<llvm::FunctionAnalysisManager> TheFAM;
static std::unique_ptr<llvm::CGSCCAnalysisManager> TheCGAM;
static std::unique_ptr<llvm::ModuleAnalysisManager> TheMAM;
static std::unique_ptr<llvm::PassInstrumentationCallbacks> ThePIC;
static std::unique_ptr<llvm::StandardInstrumentations> TheSI;

llvm::Value *LogErrorV(const char *Str) {
    LogError(Str);
    return nullptr;
}

llvm::Function *getFunction(std::string Name) {
    // First, see if the function has already been added to the current module.
    if (auto *F = TheModule->getFunction(Name))
        return F;

    // If not, check whether we can codegen the declaration from some existing
    // prototype.
    auto FI = FunctionProtos.find(Name);
    if (FI != FunctionProtos.end())
        return FI->second->codegen();

    // If no existing prototype exists, return null.
    return nullptr;
}

/// 对于所有函数定义，在入口的地方，创建VarName的AllocInst
/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
static llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function *TheFunction, llvm::StringRef VarName) {
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(llvm::Type::getDoubleTy(*TheContext), nullptr,
                             VarName);
}

/// 数字，就直接是一个APFloat类型的常量
llvm::Value *NumberExprAST::codegen() {
    // LLVM里面常量都是独立、共享的，所以直接get
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(this->value));
}

/// 变量，要判断上下文有没有
/// 为了支持可更改，所有的var改成AllocInst; 那么获得这个变量就是load指令
llvm::Value *VariableExprAST::codegen() {
    // Look this variable up in the function.
    llvm::AllocaInst *A = NamedValues[name];
    if (!A)
        return LogErrorV("Unknown variable name");

    // Load the value.
    return Builder->CreateLoad(A->getAllocatedType(), A, name.c_str());
}

/// 表达式递归的进行就好了，Builder->CreateFAdd方法里面第三个参数就是占位符，重复名字LLVM会自己处理
llvm::Value *BinaryExprAST::codegen() {
    // Special case '=' because we don't want to emit the LHS as an expression.
    if (Op == '=') {
        // Assignment requires the LHS to be an identifier.
        // This assume we're building without RTTI because LLVM builds that way by
        // default.  If you build LLVM with RTTI this can be changed to a
        // dynamic_cast for automatic error checking.
        VariableExprAST *LHSE = static_cast<VariableExprAST *>(LHS.get());
        if (!LHSE)
            return LogErrorV("destination of '=' must be a variable");
        // Codegen the RHS.
        llvm::Value *Val = RHS->codegen();
        if (!Val)
            return nullptr;

        // Look up the name.
        llvm::Value *Variable = NamedValues[LHSE->getName()];
        if (!Variable)
            return LogErrorV("Unknown variable name");

        Builder->CreateStore(Val, Variable);
        return Val;
    }
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();
    if (!L || !R)
        return nullptr;
    switch (Op) {
        case '+':
            return Builder->CreateFAdd(L, R, "addtmp");
        case '-':
            return Builder->CreateFSub(L, R, "subtmp");
        case '*':
            return Builder->CreateFMul(L, R, "multmp");
        case '<':
            L = Builder->CreateFCmpULT(L, R, "cmptmp");
            // Convert bool 0/1 to double 0.0 or 1.0
            return Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*TheContext),
                                         "booltmp");
        default:
            break;
    }
    // 可能是一个自己定义的操作符号
    llvm::Function *F = getFunction(std::string("binary") + Op);
    if (!F)
        LogErrorV("binary operator not found! " + Op);
    llvm::Value *Ops[2] = {L, R};
    return Builder->CreateCall(F, Ops, "calltmp");
}

/// 处理函数调用
llvm::Value *CallExprAST::codegen() {
    // Look up the name in the global module table.
    llvm::Function *CalleeF = TheModule->getFunction(Callee);
    if (!CalleeF)
        return LogErrorV("Unknown function referenced");

    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size())
        return LogErrorV("Incorrect # arguments passed");

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->codegen());
        if (!ArgsV.back())
            return nullptr;
    }

    return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Value *IfExprAST::codegen() {
    llvm::Value *CondV = Cond->codegen();
    if (!CondV) return nullptr;
    // 需要将condition转为一个bool
    CondV = Builder->CreateFCmpONE(CondV, llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0)), "ifcond");

    // 获得当前的函数
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
    TheFunction->print(llvm::errs());
    // 创建thenbb，并且直接将thenbb加到funciton, 此时thenbb没有前序
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(*TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*TheContext, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*TheContext, "ifcont");
    TheFunction->print(llvm::errs());
    // 创建条件分支IR，并且绑定对应的跳转，此时thenbb有了前序
    Builder->CreateCondBr(CondV, ThenBB, ElseBB);
    TheFunction->print(llvm::errs());
    // thenbb加入到结构中，虽然此时then还是空的，将thenbb设置为Builder的开始位置，那么后面加入的内容就都在ThenBB里面
    Builder->SetInsertPoint(ThenBB);
    // 注意，此时builder指向了thenbb，所以then的codegen的上下文已经换了
    llvm::Value *ThenV = Then->codegen();
    if (!ThenV)
        return nullptr;
    TheFunction->print(llvm::errs());
    Builder->CreateBr(MergeBB);
    TheFunction->print(llvm::errs());
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    // 这里是担心then本身有递归的其他表达式
    ThenBB = Builder->GetInsertBlock();

    // Emit else block. else在前边已经有了前序
    TheFunction->insert(TheFunction->end(), ElseBB);
    TheFunction->print(llvm::errs());
    Builder->SetInsertPoint(ElseBB);

    TheFunction->print(llvm::errs());

    llvm::Value *ElseV = Else->codegen();
    if (!ElseV)
        return nullptr;

    Builder->CreateBr(MergeBB);
    // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
    ElseBB = Builder->GetInsertBlock();

    // Emit merge block.
    TheFunction->insert(TheFunction->end(), MergeBB);
    TheFunction->print(llvm::errs());

    Builder->SetInsertPoint(MergeBB);
    llvm::PHINode *PN = Builder->CreatePHI(llvm::Type::getDoubleTy(*TheContext), 2, "iftmp");

    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);

    TheFunction->print(llvm::errs());
    return PN;
}

// 注意这里面的for，相当于do while，每次是先执行，然后再判断
// Output for-loop as:
//   var = alloca double
//   ...
//   start = startexpr
//   store start -> var
//   goto compare
// compare:
// endcond = endexpr
//   br endcond, body, endloop
// body:
//   body expr
//   curvar = load var
//   nextvar = curvar + step
//   store nextvar -> var
//   br compare
// outloop:
llvm::Value *ForExprAST::codegen() {
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();

    // Create an alloca for the variable in the entry block.
    llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName);

    // Emit the start code first, without 'variable' in scope.
    // 先处理loop的开始，也就是start部分，这个后面在body也会用到
    llvm::Value *StartVal = Start->codegen();
    if (!StartVal)
        return nullptr;
    // Store the value into the alloca.
    Builder->CreateStore(StartVal, Alloca);
    // Within the loop, the variable is defined equal to the PHI node.  If it
    // shadows an existing variable, we have to restore it, so save it now.
    llvm::AllocaInst *OldVal = NamedValues[VarName];
    NamedValues[VarName] = Alloca;
    TheFunction->print(llvm::errs());

    // 创建compare
    llvm::BasicBlock *LoopCompare = llvm::BasicBlock::Create(*TheContext, "compare", TheFunction);
    // loop自己
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(*TheContext, "loop", TheFunction);
    // Create the "after loop" block and insert it.
    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(*TheContext, "afterloop", TheFunction);
    TheFunction->print(llvm::errs());

    Builder->CreateBr(LoopCompare);
    TheFunction->print(llvm::errs());

    Builder->SetInsertPoint(LoopCompare);
    // Compute the end condition.
    llvm::Value *EndCond = End->codegen();
    if (!EndCond)
        return nullptr;
    // Convert condition to a bool by comparing non-equal to 0.0.
    EndCond = Builder->CreateFCmpONE(
        EndCond, llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0)), "loopcond");
    // Insert the conditional branch into the end of LoopEndBB.
    Builder->CreateCondBr(EndCond, LoopBB, AfterBB);
    TheFunction->print(llvm::errs());


    // Start insertion in LoopBB.
    Builder->SetInsertPoint(LoopBB);

    // Emit the body of the loop.  This, like any other expr, can change the
    // current BB.  Note that we ignore the value computed by the body, but don't
    // allow an error.
    if (!Body->codegen())
        return nullptr;

    // Emit the step value.
    llvm::Value *StepVal = nullptr;
    if (Step) {
        StepVal = Step->codegen();
        if (!StepVal)
            return nullptr;
    } else {
        // If not specified, use 1.0.
        StepVal = llvm::ConstantFP::get(*TheContext, llvm::APFloat(1.0));
    }
    TheFunction->print(llvm::errs());
    // Reload, increment, and restore the alloca.  This handles the case where
    // the body of the loop mutates the variable.
    llvm::Value *CurVar = Builder->CreateLoad(Alloca->getAllocatedType(), Alloca, VarName.c_str());
    llvm::Value *NextVar = Builder->CreateFAdd(CurVar, StepVal, "nextvar");
    Builder->CreateStore(NextVar, Alloca);
    Builder->CreateBr(LoopCompare);


    // Any new code will be inserted in AfterBB.
    Builder->SetInsertPoint(AfterBB);

    // Restore the unshadowed variable.
    if (OldVal)
        NamedValues[VarName] = OldVal;
    else
        NamedValues.erase(VarName);

    // for expr always returns 0.0.
    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*TheContext));
}

llvm::Value *UnaryExprAST::codegen() {
    llvm::Value *OperandV = Operand->codegen();
    if (!OperandV)
        return nullptr;

    llvm::Function *F = getFunction(std::string("unary") + Opcode);
    if (!F)
        return LogErrorV("Unknown unary operator");

    return Builder->CreateCall(F, OperandV, "unop");
}

llvm::Value *VarExprAST::codegen() {
    std::vector<llvm::AllocaInst *> OldBindings;

    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();

    // Register all variables and emit their initializer.
    for (unsigned i = 0, e = VarNames.size(); i != e; ++i) {
        const std::string &VarName = VarNames[i].first;
        ExprAST *Init = VarNames[i].second.get();

        // Emit the initializer before adding the variable to scope, this prevents
        // the initializer from referencing the variable itself, and permits stuff
        // like this:
        //  var a = 1 in
        //    var a = a in ...   # refers to outer 'a'.
        llvm::Value *InitVal;
        if (Init) {
            InitVal = Init->codegen();
            if (!InitVal)
                return nullptr;
        } else { // If not specified, use 0.0.
            InitVal = llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0));
        }

        llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
        Builder->CreateStore(InitVal, Alloca);

        // Remember the old variable binding so that we can restore the binding when
        // we unrecurse.
        OldBindings.push_back(NamedValues[VarName]);

        // Remember this binding.
        NamedValues[VarName] = Alloca;
    }

    // Codegen the body, now that all vars are in scope.
    llvm::Value *BodyVal = Body->codegen();
    if (!BodyVal)
        return nullptr;

    // Pop all our variables from scope.
    for (unsigned i = 0, e = VarNames.size(); i != e; ++i)
        NamedValues[VarNames[i].first] = OldBindings[i];

    // Return the body computation.
    return BodyVal;
}

/// 函数签名：就是一个类型表示，所以是返回值+参数类型的组合
llvm::Function *PrototypeAST::codegen() {
    // Make the function type:  double(double,double) etc.
    std::vector<llvm::Type *> Doubles(Args.size(), llvm::Type::getDoubleTy(*TheContext));
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(*TheContext), Doubles, false);

    // externallinkage表示函数可能是外部的
    // 同时，这个函数注册到module里面
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg: F->args())
        Arg.setName(Args[Idx++]);

    return F;
}

llvm::Function *FunctionAST::codegen() {
    auto &P = *Proto;
    FunctionProtos[Proto->getName()] = std::move(Proto);
    llvm::Function *TheFunction = getFunction(P.getName());

    // First, check for an existing function from a previous 'extern' declaration.
    if (!TheFunction)
        return nullptr;

    // If this is an operator, install it.
    if (P.isBinaryOp())
        BinopPrecedence[P.getOperatorName()] = P.getBinaryPrecedence();

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto &Arg : TheFunction->args()) {
        // Create an alloca for this variable.
        llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName());

        // Store the initial value into the alloca.
        Builder->CreateStore(&Arg, Alloca);

        // Add arguments to variable symbol table.
        NamedValues[std::string(Arg.getName())] = Alloca;
    }

    if (llvm::Value *RetVal = Body->codegen()) {
        // Finish off the function.
        Builder->CreateRet(RetVal);

        // Validate the generated code, checking for consistency.
        verifyFunction(*TheFunction);

        // Run the optimizer on the function.
        TheFPM->run(*TheFunction, *TheFAM);

        return TheFunction;
    }

    // Error reading body, remove function.
    TheFunction->eraseFromParent();

    if (P.isBinaryOp())
        BinopPrecedence.erase(P.getOperatorName());
    return nullptr;
}

//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//

/// 初始化
static void InitializeModuleAndManagers() {
    // Open a new context and module.
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("my cool jit", *TheContext);

    // Create a new builder for the module.
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

    // Create new pass and analysis managers. 不同维度的优化
    TheFPM = std::make_unique<llvm::FunctionPassManager>();
    TheLAM = std::make_unique<llvm::LoopAnalysisManager>();
    TheFAM = std::make_unique<llvm::FunctionAnalysisManager>();
    TheCGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
    TheMAM = std::make_unique<llvm::ModuleAnalysisManager>();
    ThePIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
    TheSI = std::make_unique<llvm::StandardInstrumentations>(*TheContext,
                                                       /*DebugLogging*/ true);
    TheSI->registerCallbacks(*ThePIC, TheMAM.get());

    // Add transform passes. transform会更改IR，但是analysis只会分析
    // Promote allocas to registers.
    TheFPM->addPass(llvm::PromotePass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->addPass(llvm::InstCombinePass());
    // Reassociate expressions.
    TheFPM->addPass(llvm::ReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->addPass(llvm::GVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->addPass(llvm::SimplifyCFGPass());

    // Register analysis passes used in these transform passes.
    llvm::PassBuilder PB;
    PB.registerModuleAnalyses(*TheMAM);
    PB.registerFunctionAnalyses(*TheFAM);
    PB.crossRegisterProxies(*TheLAM, *TheFAM, *TheCGAM, *TheMAM);
}

/// 后面就可以用一个驱动来处理parser
static void HandleDefinition() {
    if (auto R = ParseDefinition()) {
        std::cout << "Parsed a function definition.\n";
        std::cout << R->printToStr() << std::endl;
        if (auto *FnIR = R->codegen()) {
            FnIR->print(llvm::errs());
        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleExtern() {
    if (auto R = ParseExtern()) {
        std::cout << "Parsed an extern\n";
        std::cout << R->printToStr() << std::endl;
        if (auto *FnIR = R->codegen()) {
            FnIR->print(llvm::errs());
        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (auto R = ParseTopLevelExpr()) {
        std::cout << "Parsed a top-level expr\n";
        std::cout << R->printToStr() << std::endl;
        if (auto *FnIR = R->codegen()) {
            FnIR->print(llvm::errs());
            // Remove the anonymous expression.
            FnIR->eraseFromParent();
        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}
/// top ::= definition | external | expression | ';'
static void MainLoop() {
    while (InputIndex < Input.size()) {
        switch (CurTok) {
            case tok_eof:
                return;
            case ';':
                getNextToken();
                break;
            case tok_def:
                HandleDefinition();
                break;
            case tok_extern:
                HandleExtern();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
    }
}
//===----------------------------------------------------------------------===//
// Driver end
//===----------------------------------------------------------------------===//

int main() {
    std::cout << "Hello World!" << std::endl;
    // Input = "a + 1 * 4 + defts";
    // Input = "a+1*(45+b*cd)+6*7";
    // Input = "def foo() (45+12)*15"; //-> 这里会直接优化返回855
    // Input = "def add(a b) a + b";
    // Input = "add(1,2+3)"; //
    Input = "def addTwo(a) a+1+1;" // -> 这里不会直接优化：？1+1+a就会
            "def squire(a) a*a; "
            "def foo(a b) squire(a) + 2*a*b + squire(b);"
            "def test(x) (1+2+x)*(x+(1+2));"
            "def fib(x) "
            "   if x < 3 then"
            "       squire(x)"
            "   else"
            "       fib (x-1) + squire(x)"
            "extern putchard(char);"
            "def printstar(n)"
            "   for i=1, i < n, 2.0 in"
            "       putchard(42);"
            "def unary!(v)"
            "   if v then"
            "       0"
            "   else"
            "       1;"
            "def binary| 5(L R)"
            "   if L then"
            "       1"
            "   else if R then"
            "       1"
            "   else"
            "       0;"
            "extern printd(a);"
            "def testunary(a)"
            "   !a;"
            "def testbinary(a b)"
            "   a | b;"
            "def binary : 1 (x y) y;" // -> 取y这个值
            "def mysquire(a)"
            "   if a < 5 then"
            "       a * a"
            "   else"
            "       a;"
            "def specialsum(x)"
            "   var res in (for i = 1, i < x, 1.0 in res = res + mysquire(i)) : res"
            ;
    getNextToken(); // 启动
    // Make the module, which holds all the code.
    InitializeModuleAndManagers();
    MainLoop();
    // Print out all of the generated code.
    std::cout << "Dump: IR" << std::endl;
    TheModule->print(llvm::errs(), nullptr);

    // --- gen object file --- //
    // Initialize the target registry etc.
    // llvm::InitializeAllTargetInfos();
    // llvm::InitializeAllTargets();
    // llvm::InitializeAllTargetMCs();
    // llvm::InitializeAllAsmParsers();
    // llvm::InitializeAllAsmPrinters();
    // LLVMInitializeARMTarget();
    // LLVMInitializeARMTargetInfo();
    // LLVMInitializeARMTargetMC();
    // LLVMInitializeARMAsmParser();
    // LLVMInitializeARMAsmPrinter();
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    llvm::Triple TargetTriple(llvm::sys::getDefaultTargetTriple());
    TheModule->setTargetTriple(TargetTriple);
    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        llvm::errs() << Error;
        return 1;
    }

    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto TheTargetMachine = Target->createTargetMachine(
        TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_);

    TheModule->setDataLayout(TheTargetMachine->createDataLayout());

    auto Filename = "output.o";
    std::error_code EC;
    llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        return 1;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CodeGenFileType::ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return 1;
    }

    pass.run(*TheModule);
    dest.flush();


    std::cout << "finish" << std::endl;
    return 0;
}

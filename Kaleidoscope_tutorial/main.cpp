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
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

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
        return tok_identifier;
    }
    // check number
    if (isdigit(LastChar)) {
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
    }
}

/// E: 解析二元表达式，这个会复杂一些，因为涉及到优先级
/// 所以，我们先注册符号的优先级，在解析的过程中，一定是形如 Lhs op Rhs [op2 T]
/// （1）如果后面的符号op2优先级高，那么就以当前的Rhs，作为下一个的Lhs进行，结果一起作为结果
/// 也就是：Lhs op (Lhs=Rhs, op2, Rhs=T)
/// (2) 反之op2低，那么就先合并Lhs和Rhs，一起作为下一个的Lhs，(Lhs=Lhs, op, Rhs=Rhs) op2 T
static std::unordered_map<char, int> BinopPrecedence = {
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
        auto RHS = ParsePrimary();
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
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;
    return ParseBinOpRHS(0, std::move(LHS));
}

/// G: 函数声明
/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
    if (CurTok != tok_identifier) return LogErrorP("Expected function name in prototype");
    std::string FnName = Id;
    getNextToken();
    if (CurTok != '(') return LogErrorP("expected ')'");
    std::vector<std::string> ArgNames;
    while (getNextToken() == tok_identifier)
        ArgNames.push_back(Id);
    if (CurTok != ')')
        return LogErrorP("Expected ')' in prototype");

    // success.
    getNextToken(); // eat ')'.

    return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
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
static std::unordered_map<std::string, llvm::Value *> NamedValues; // 记录当前符号表

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

/// 数字，就直接是一个APFloat类型的常量
llvm::Value *NumberExprAST::codegen() {
    // LLVM里面常量都是独立、共享的，所以直接get
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(this->value));
}

/// 变量，要判断上下文有没有
llvm::Value *VariableExprAST::codegen() {
    llvm::Value *V = NamedValues[this->name];
    //
    if (!V)
        LogErrorV(("Unknown variable name: " + this->name).c_str());
    return V;
}

/// 表达式递归的进行就好了，Builder->CreateFAdd方法里面第三个参数就是占位符，重复名字LLVM会自己处理
llvm::Value *BinaryExprAST::codegen() {
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
            return LogErrorV("invalid binary operator");
    }
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

    // 创建thenbb，并且直接将thenbb加到funciton的最后
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(*TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*TheContext, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*TheContext, "ifcont");
    // 创建条件分支IR，并且绑定对应的跳转
    Builder->CreateCondBr(CondV, ThenBB, ElseBB);
    TheFunction->print(llvm::errs());
    // thenbb加入到结构中，虽然此时then还是空的，将thenbb设置为Builder的开始位置，那么后面加入的内容就都在ThenBB里面
    Builder->SetInsertPoint(ThenBB);
    // 注意，此时builder指向了thenbb，所以then的codegen的上下文已经换了
    llvm::Value *ThenV = Then->codegen();
    if (!ThenV)
        return nullptr;

    Builder->CreateBr(MergeBB);
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    // 这里是担心then本身有递归的其他表达式
    ThenBB = Builder->GetInsertBlock();

    // Emit else block.
    TheFunction->insert(TheFunction->end(), ElseBB);
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
    // First, check for an existing function from a previous 'extern' declaration.
    llvm::Function *TheFunction = TheModule->getFunction(Proto->getName());

    if (!TheFunction)
        TheFunction = Proto->codegen();

    if (!TheFunction)
        return nullptr;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        NamedValues[std::string(Arg.getName())] = &Arg;

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
            "       fib (x-1) + squire(x)";
    getNextToken(); // 启动
    // Make the module, which holds all the code.
    InitializeModuleAndManagers();
    MainLoop();
    // Print out all of the generated code.
    TheModule->print(llvm::errs(), nullptr);
    std::cout << "finish" << std::endl;
    return 0;
}

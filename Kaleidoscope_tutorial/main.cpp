//
// Created by zuxinggu on 2025/8/4.
//

#include "ast.h"
#include <iostream>
#include <string>
#include <unordered_map>

static std::string Input;
static int InputIndex = 0;

// return tokens [0-255] to represent unknown character, otherwise known tokens
enum Token {
    tok_eof = -1,

    // key word
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,
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
        // special id, def
        if (Id == "def") {
            return tok_def;
        }
        if (Id == "extern") {
            return tok_extern;
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
/// end lexer

/// parser
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

/// 后面就可以用一个驱动来处理parser
static void HandleDefinition() {
    if (auto R = ParseDefinition()) {
        std::cout << "Parsed a function definition.\n";
        std::cout << R->printToStr() << std::endl;
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleExtern() {
    if (auto R = ParseExtern()) {
        std::cout << "Parsed an extern\n";
        std::cout << R->printToStr() << std::endl;
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

/// end parser
int main() {
    std::cout << "Hello World!" << std::endl;
    // Input = "a + 1 * 4 + defts";
    Input = "a+1*(45+b*cd)+6*7";
    // Input = "(45+12)*15";
    // Input = "def add(a b) a + b";
    // Input = "add(1,2+3)";
    getNextToken(); // 启动
    MainLoop();
    std::cout << "finish" << std::endl;
    return 0;
}

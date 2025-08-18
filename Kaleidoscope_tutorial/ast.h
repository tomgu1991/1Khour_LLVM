//
// Created by zuxinggu on 2025/8/18.
//

#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

/// 所有表达式AST的基础。
/// 根据spec：
/// 二元、基础表达式（变量、调用 ｜ 数值 ｜ 括号表达式）
/// 二元可以和括号合并，用树孩子的方式记录优先级
/// 所以，一共有四种子类型
class ExprAST {
public:
    virtual ~ExprAST() = default;
    virtual std::string printToStr() = 0;
};

class NumberExprAST : public ExprAST {
    double value;
public:
    NumberExprAST(double value) : value(value) {}
    std::string printToStr() override { return std::to_string(value); }
};

class VariableExprAST : public ExprAST {
    std::string name;
public:
    VariableExprAST(std::string name) : name(name) {}
    std::string printToStr() override { return name; }
};

class BinaryExprAST : public ExprAST {
    char Op; // 运算符号
    std::unique_ptr<ExprAST> LHS, RHS; // 左右操作数
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> L, std::unique_ptr<ExprAST> R) :
        Op(op), LHS(std::move(L)), RHS(std::move(R)) {}
    std::string printToStr() override {
        std::string R = "";
        R += "[L: (" + LHS->printToStr() + ")";
        if (RHS) {
            R = R + "; op = " + Op + "; \n    R: (" + RHS->printToStr() + ")]";
        }
        return R;
    }
};

class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args) :
        Callee(Callee), Args(std::move(Args)) {}
    std::string printToStr() override {
        std::string R = "";
        for (auto &E : Args)
            R += E->printToStr() + ", ";
        return "Call: " + Callee + "(" + R + ")";
    }
};

/// 函数声明
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;
public:
    PrototypeAST(std::string name, std::vector<std::string> args) : Name(name), Args(std::move(args)) {}
    const std::string& getName() const { return Name; }
    std::string printToStr() {
        std::string R = "";
        for (auto &E : Args)
            R += E + ", ";
        return Name + "(" + R + ")";
    }
};

/// 函数定义
class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;
public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body) :
        Proto(std::move(Proto)), Body(std::move(Body)) {}
    std::string printToStr() {
        return "FunctionDef: " + Proto->printToStr() + "\nBody: " + Body->printToStr();
    }
};

#endif //AST_H

//
// Created by zuxinggu on 2025/8/18.
//

#ifndef AST_H
#define AST_H

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
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
    virtual llvm::Value *codegen() = 0;
};

class NumberExprAST : public ExprAST {
    double value;
public:
    NumberExprAST(double value) : value(value) {}
    std::string printToStr() override { return std::to_string(value); }
    llvm::Value *codegen() override;
};

class VariableExprAST : public ExprAST {
    std::string name;
public:
    VariableExprAST(std::string name) : name(name) {}
    std::string printToStr() override { return name; }
    llvm::Value *codegen() override;
    std::string getName() { return name; }
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
    llvm::Value *codegen() override;
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
    llvm::Value *codegen() override;
};

class IfExprAST : public ExprAST {
    std::unique_ptr<ExprAST> Cond, Then, Else;
public:
    IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then,
            std::unique_ptr<ExprAST> Else)
    : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

    std::string printToStr() override {
        std::string R = "IfExpr:\n";
        R += "Con: " + Cond->printToStr() + "\n";
        if (Then) {
            R += "Then: " + Then->printToStr() + "\n";
        }
        if (Else) {
            R += "Else: " + Else->printToStr() + "\n";
        }
        return R;
    }
    llvm::Value *codegen() override;
};

/// ForExprAST - Expression class for for/in.
class ForExprAST : public ExprAST {
    std::string VarName;
    std::unique_ptr<ExprAST> Start, End, Step, Body;

public:
    ForExprAST(const std::string &VarName, std::unique_ptr<ExprAST> Start,
               std::unique_ptr<ExprAST> End, std::unique_ptr<ExprAST> Step,
               std::unique_ptr<ExprAST> Body)
      : VarName(VarName), Start(std::move(Start)), End(std::move(End)),
        Step(std::move(Step)), Body(std::move(Body)) {}

    std::string printToStr() override {
        std::string R = "";
        R += "ForExpr: \nVarName: " + VarName + "\n";
        if (Start) {
            R += "Start: " + Start->printToStr() + "\n";
        }
        if (End) {
            R += "End: " + End->printToStr() + "\n";
        }
        if (Step) {
            R += "Step: " + Step->printToStr() + "\n";
        }
        if (Body) {
            R += "Body: " + Body->printToStr() + "\n";
        }
        return R;
    }
    llvm::Value *codegen() override;
};

/// Unary表达式
/// UnaryExprAST - Expression class for a unary operator.
class UnaryExprAST : public ExprAST {
    char Opcode;
    std::unique_ptr<ExprAST> Operand;

public:
    UnaryExprAST(char Opcode, std::unique_ptr<ExprAST> Operand)
      : Opcode(Opcode), Operand(std::move(Operand)) {}
    std::string printToStr() override {
        std::string R = Opcode + "(" + Operand->printToStr() + ")";
        return R;
    }
    llvm::Value *codegen() override;
};

/// VarExprAST - Expression class for var/in
class VarExprAST : public ExprAST {
    std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;
    std::unique_ptr<ExprAST> Body;

public:
    VarExprAST(std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames, std::unique_ptr<ExprAST> Body) :
        VarNames(std::move(VarNames)), Body(std::move(Body)) {}

    std::string printToStr() override {
        std::string R = "var ";
        for (auto &V : VarNames) {
            R += V.first + ", ";
        }
        return R + "(" + Body->printToStr() + ")";
    }
    llvm::Value *codegen() override;
};

/// 函数声明
/// 如果是自己定义的，那么把函数命名为unary+op/binary+op
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;
    bool IsOperator;
    unsigned Precedence; // Precedence if a binary op.
public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args, bool IsOperator = false, unsigned Prec = 0) :
        Name(Name), Args(std::move(Args)), IsOperator(IsOperator), Precedence(Prec) {}
    const std::string& getName() const { return Name; }
    std::string printToStr() {
        std::string R = "";
        for (auto &E : Args)
            R += E + ", ";
        std::string S = "";
        if (IsOperator) {
            S = "Op=" + Name[Name.size() - 1];
            S += ", Opd=" + Args.size();
        }
        return Name + "(" + R + ")" + S;
    }
    llvm::Function *codegen();

    bool isUnaryOp() const { return IsOperator && Args.size() == 1; }
    bool isBinaryOp() const { return IsOperator && Args.size() == 2; }

    char getOperatorName() const {
        assert(isUnaryOp() || isBinaryOp());
        return Name[Name.size() - 1];
    }

    unsigned getBinaryPrecedence() const { return Precedence; }
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
    llvm::Function *codegen();
};

#endif //AST_H

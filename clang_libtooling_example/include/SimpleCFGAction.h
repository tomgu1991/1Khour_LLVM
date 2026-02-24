//
// Created by zuxinggu on 2024/8/29.
//

#ifndef SIMPLECFGACTION_H
#define SIMPLECFGACTION_H

#include "clang/AST/RecursiveASTVisitor.h"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <iostream>
#include <string>
#include <unordered_set>

class SimpleCFGVisitor : public clang::RecursiveASTVisitor<SimpleCFGVisitor> {
public:
  explicit SimpleCFGVisitor(clang::ASTContext *Context) : Context(Context) {}

  bool shouldTraversePostOrder() const { return false; }

  bool TraverseIfStmt(clang::IfStmt *S, DataRecursionQueue *Queue = nullptr);
  // bool TraverseIfStmt(clang::IfStmt *S, DataRecursionQueue *Queue = nullptr) {
  //   clang::PrintingPolicy policy(Context->getLangOpts());
  //   policy.adjustForCPlusPlus();
  //
  //   // Print the expression to a string
  //   std::string exprStr;
  //   llvm::raw_string_ostream stream(exprStr);
  //   S->getCond()->printPretty(stream, nullptr, policy);
  //   stream.flush();
  //   std::cout << INDENT << "IfStmt: "
  //             << "Cond = " << exprStr << std::endl;
  //   INDENT = INDENT + INDENT_BASE;
  //   bool result = RecursiveASTVisitor::TraverseIfStmt(S);
  //   if (result) {
  //     INDENT = INDENT.substr(0, INDENT.size() - 4);
  //   }
  //
  //   return result;
  // }

  bool VisitFunctionDecl(clang::FunctionDecl *F) {
    if (F->getDefinition() == nullptr) {
      return true;
    }
    auto *Function = F->getDefinition();
    VisitedFunctions.insert(Function);
    std::cout << "Processing " << Function->getQualifiedNameAsString() << std::endl;
    std::cout << INDENT << "Name: " << Function->getQualifiedNameAsString() << std::endl;
    return true;
  }

  bool VisitCallExpr(clang::CallExpr *Expr) {
    std::cout << INDENT << "CXXMemberCall: " << Expr->getDirectCallee()->getQualifiedNameAsString()
              << ", args = " << Expr->getNumArgs() << std::endl;
    return true;
  }

private:
  clang::ASTContext *Context;
  std::unordered_set<clang::FunctionDecl *> VisitedFunctions;
  std::string INDENT = "    ";
  const std::string INDENT_BASE = "    ";
};

class FuncCollector : public clang::RecursiveASTVisitor<FuncCollector> {
public:
  std::unordered_set<clang::FunctionDecl *> Funcs;
  explicit FuncCollector(clang::ASTContext *Context) : Context(Context) {}
  bool VisitFunctionDecl(clang::FunctionDecl *F) {
    if (F->getDefinition() == nullptr) {
      return true;
    }
    auto *Function = F->getDefinition();
    Funcs.insert(Function);
    return true;
  }

private:
  clang::ASTContext *Context;
};

class SimpleCFGConsumer : public clang::ASTConsumer {
public:
  explicit SimpleCFGConsumer(clang::ASTContext *Context, llvm::StringRef InFile)
      : FuncVisitor(Context), InFile(InFile) {}
  void HandleTranslationUnit(clang::ASTContext &Context) override {
    std::cout << "Processing: " << InFile.str() << std::endl;
    std::cout << "AST: " << std::endl;
    Context.getTranslationUnitDecl()->dump();
    FuncVisitor.TraverseDecl(Context.getTranslationUnitDecl());
    for (auto *Func : FuncVisitor.Funcs) {
      SimpleCFGVisitor CFGVisitor(&Context);
      CFGVisitor.TraverseFunctionDecl(Func);
    }
  }

public:
  FuncCollector FuncVisitor;
  llvm::StringRef InFile;
};

class SimpleCFGAction : public clang::ASTFrontendAction {
public:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                        llvm::StringRef InFile) override {
    return std::make_unique<SimpleCFGConsumer>(&CI.getASTContext(), InFile);
  }
};

#endif // SIMPLECFGACTION_H

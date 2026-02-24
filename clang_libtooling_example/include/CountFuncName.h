//
// Created by zuxinggu on 2024/8/29.
//

#ifndef COUNTFUNCNAME_H
#define COUNTFUNCNAME_H

#include "clang/AST/RecursiveASTVisitor.h"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <iostream>
#include <string>

#define LOG_MSG "gzx-count-func:"

// visitor to collect func name
class CountFuncVisitor : public clang::RecursiveASTVisitor<CountFuncVisitor> {
public:
  explicit CountFuncVisitor(clang::ASTContext *Context) : Context(Context) {}
  bool VisitFunctionDecl(clang::FunctionDecl *D) {
    std::string Name = D->getQualifiedNameAsString();
    std::cout << LOG_MSG << "visit func: " << Name << std::endl;
    return true;
  }

private:
  clang::ASTContext *Context;
};

// consumer to call visitor
class CountFuncConsumer : public clang::ASTConsumer {
public:
  explicit CountFuncConsumer(clang::ASTContext *Context) : Visitor(Context) {}
  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Context.getTranslationUnitDecl()->dump();
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  CountFuncVisitor Visitor;
};

// front end action to visit AST and count func num
class CountFuncName : public clang::ASTFrontendAction {
public:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler,
                                                        llvm::StringRef InFile) override {
    return std::make_unique<CountFuncConsumer>(&Compiler.getASTContext());
  }
};

#endif // COUNTFUNCNAME_H

//
// Created by zuxinggu on 2024/9/2.
//

#include "SimpleCFGAction.h"

bool SimpleCFGVisitor::TraverseIfStmt(clang::IfStmt *S, DataRecursionQueue *Queue) {
  clang::PrintingPolicy policy(Context->getLangOpts());
  policy.adjustForCPlusPlus();

  // Print the expression to a string
  std::string exprStr;
  llvm::raw_string_ostream stream(exprStr);
  S->getCond()->printPretty(stream, nullptr, policy);
  stream.flush();
  std::cout << INDENT << "IfStmt: "
            << "Cond = " << exprStr << std::endl;
  INDENT = INDENT + INDENT_BASE;
  bool result = RecursiveASTVisitor::TraverseIfStmt(S);
  if (result) {
    INDENT = INDENT.substr(0, INDENT.size() - 4);
  }

  return result;
}
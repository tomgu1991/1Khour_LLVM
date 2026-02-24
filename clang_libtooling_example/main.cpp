//
// Created by guzuxing on 2024/1/2.
//

#include "CountFuncName.h"
#include "SimpleCFGAction.h"

#include "clang/Driver/Options.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/JSON.h"
#include <iostream>
#include <string>

using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using llvm::json::Object;
using llvm::json::Array;
using llvm::json::Value;

// Apply a custom category to all command-line options so that they are the only
// ones displayed.
static llvm::cl::OptionCategory MyToolCategory("example options");
static const opt::OptTable &Options = clang::driver::getDriverOptTable();
static cl::opt<bool> COUNT_FUNC_NAME("count-func-name", cl::desc("count function names"),
                                     cl::cat(MyToolCategory));
static cl::opt<bool> SIMPLE_CFG("simple-cfg", cl::desc("simple config"), cl::cat(MyToolCategory));

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
int main(int argc, const char **argv) {
  std::cout << "start" << std::endl;
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
  if (!ExpectedParser) {
    // Fail gracefully for unsupported options.
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
  for (auto &Name : OptionsParser.getSourcePathList()) {
    std::cout << "file name: " << Name << std::endl;
  }
  std::cout << "run front end anction: CountFuncName" << std::endl;
  if (COUNT_FUNC_NAME) {
    std::cout << "CountFuncName" << std::endl;
    Tool.run(newFrontendActionFactory<CountFuncName>().get());
  }

  if (SIMPLE_CFG) {
    std::cout << "Simple CFG" << std::endl;
    Tool.run(newFrontendActionFactory<SimpleCFGAction>().get());
  }
  std::cout << "finish" << std::endl;

  llvm::json::Object root;
  root["status"] = 0;
  
  llvm::json::Array resultArray;
  for (int i = 0; i < 4; i++) {
    Object obj;
    obj["file"] = "test.cpp";
    obj["id"] = 1;
    resultArray.push_back(std::move(obj));
  }
  root["result"] = std::move(resultArray);
  
  std::string jsonStr;
  llvm::raw_string_ostream os(jsonStr);
  os << llvm::json::Value(std::move(root));
  std::cout << jsonStr << std::endl;

  return 0;
}
static cl::extrahelp MoreHelp("usage: example [--count-func-name] test.cpp --\n"
                              "    example [--simple-cfg] test.cpp --\n");
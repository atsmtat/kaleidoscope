#pragma once

#include "ast.h"
#include "visitor.h"
#include "llvm/IR/LLVMContext.h"
#include <deque>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <unordered_map>

class Codegen : public Visitor {
public:
  Codegen() : builder_(llvmContext_) {
    theModule_ =
        std::make_unique<llvm::Module>("my first module", llvmContext_);
    setupFunctionPassManager();
  }
  ~Codegen() = default;

  void setupFunctionPassManager();

  void visit(ExprNode &exprNode) override;
  void visit(NumberExprNode &numExpr) override;
  void visit(VariableExprNode &varExpr) override;
  void visit(BinaryExprNode &binExpr) override;
  void visit(CallExprNode &callExpr) override;
  void visit(IfElseExprNode &ifelseExpr) override;
  void visit(FunctionNode &funcNode) override;

  void printIR(const char *msg) const;
  void printModule() const;

private:
  llvm::LLVMContext llvmContext_;
  llvm::IRBuilder<> builder_;
  std::unique_ptr<llvm::Module> theModule_;
  std::unordered_map<std::string, llvm::Value *> symTable_;
  std::deque<llvm::Value *> valStack_;
  llvm::Function *lastFn_;

  std::unique_ptr<llvm::legacy::FunctionPassManager> theFPM_;
};

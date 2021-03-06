#include "codegen.h"
#include <exception>
#include <iostream>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <sstream>

using namespace llvm;

void logError(const std::string &err) {
  std::cerr << "error: " << err << std::endl;
}

void Codegen::setupFunctionPassManager() {
  theFPM_ = std::make_unique<legacy::FunctionPassManager>(theModule_.get());

  // "peephole" and bit-twiddling optimizations
  theFPM_->add(createInstructionCombiningPass());
  // Reassociate expressions
  theFPM_->add(createReassociatePass());
  // Common Subexpression Elimination
  theFPM_->add(createGVNPass());
  // Simplify CFG (delete unreachable blocks, etc.)
  theFPM_->add(createCFGSimplificationPass());

  theFPM_->doInitialization();
}

void Codegen::visit(ExprNode &exprNode) { assert(false); }

void Codegen::visit(NumberExprNode &numExpr) {
  valStack_.emplace_front(
      ConstantFP::get(llvmContext_, APFloat(numExpr.num())));
}

void Codegen::visit(VariableExprNode &varExpr) {
  const std::string varName = varExpr.varName();
  if (symTable_.find(varName) == symTable_.end()) {
    std::ostringstream ostr;
    ostr << "unknown variable '" << varName << "'";
    logError(ostr.str());
    return;
  }
  valStack_.emplace_front(symTable_[varName]);
}

void Codegen::visit(BinaryExprNode &binExpr) {
  binExpr.lhs()->accept(*this);
  binExpr.rhs()->accept(*this);
  if (valStack_.size() < 2) {
    return;
  }

  Value *lhs = valStack_.front();
  valStack_.pop_front();
  Value *rhs = valStack_.front();
  valStack_.pop_front();

  Value *result = nullptr;
  switch (binExpr.op()) {
  case BinaryExprNode::Op::plus:
    result = builder_.CreateFAdd(lhs, rhs, "addtmp");
    break;
  case BinaryExprNode::Op::minus:
    result = builder_.CreateFSub(lhs, rhs, "subtmp");
    break;
  case BinaryExprNode::Op::mul:
    result = builder_.CreateFMul(lhs, rhs, "multmp");
    break;
  default:
    logError("error: invalid binary operator");
  }

  valStack_.emplace_front(result);
}

void Codegen::visit(CallExprNode &callExpr) {
  // Lookup called function name in llvm module table
  Function *func = theModule_->getFunction(callExpr.callee());
  if (!func) {
    logError("error: called unknown function");
    return;
  }

  if (func->arg_size() != callExpr.args().size()) {
    logError("error: incorrect number of args passed in function call");
    return;
  }

  std::vector<Value *> argsV;
  for (const auto &arg : callExpr.args()) {
    arg->accept(*this);
    if (valStack_.empty()) {
      return;
    }
    argsV.emplace_back(valStack_.front());
    valStack_.pop_front();
  }

  valStack_.emplace_front(
      builder_.CreateCall(func, std::move(argsV), "calltmp"));
}

void Codegen::visit(IfElseExprNode &ifelseExpr) {
  ifelseExpr.condExpr()->accept(*this);
  if (valStack_.empty()) {
    return;
  }

  // Convert cond expr to bool by comparing with 0.0 (x != 0.0)
  Value *condVal = valStack_.front();
  valStack_.pop_front();
  condVal = builder_.CreateFCmpONE(
      condVal, ConstantFP::get(llvmContext_, APFloat(0.0)), "ifcond");

  // Get Function in which we want to add BB for then, else and ifcont.
  Function *fun = builder_.GetInsertBlock()->getParent();

  // Create BB for then and insert it at the end of fun
  BasicBlock *thenBB = BasicBlock::Create(llvmContext_, "then", fun);

  // Create BB for else and merge (if cont..) but don't insert them into
  // fun yet, as codegen in "then" might insert BBs after thenBB.
  BasicBlock *elseBB = BasicBlock::Create(llvmContext_, "else");
  BasicBlock *ifContBB = BasicBlock::Create(llvmContext_, "ifcont");

  // Create conditional branch
  builder_.CreateCondBr(condVal, thenBB, elseBB);

  // Emit then code in thenBB
  builder_.SetInsertPoint(thenBB);
  ifelseExpr.thenExpr()->accept(*this);
  if (valStack_.empty()) {
    return;
  }
  Value *thenVal = valStack_.front();
  valStack_.pop_front();
  builder_.CreateBr(ifContBB);
  // codegen for then could change the current block,
  // get then predecessor for phi
  BasicBlock *thenPredBB = builder_.GetInsertBlock();

  // Emit else code in elseBB
  // Insert elseBB into fun
  fun->getBasicBlockList().push_back(elseBB);
  builder_.SetInsertPoint(elseBB);
  ifelseExpr.elseExpr()->accept(*this);
  if (valStack_.empty()) {
    return;
  }
  Value *elseVal = valStack_.front();
  valStack_.pop_front();
  builder_.CreateBr(ifContBB);
  // codegen for else could change the current block,
  // get else predecessor for phi
  BasicBlock *elsePredBB = builder_.GetInsertBlock();

  // Emit if cont. code
  fun->getBasicBlockList().push_back(ifContBB);
  builder_.SetInsertPoint(ifContBB);
  PHINode *phiNode =
      builder_.CreatePHI(Type::getDoubleTy(llvmContext_), 2, "iftmp");
  phiNode->addIncoming(thenVal, thenPredBB);
  phiNode->addIncoming(elseVal, elsePredBB);

  valStack_.emplace_front(phiNode);
}

void Codegen::visit(FunctionNode &funcNode) {
  Function *fun = theModule_->getFunction(funcNode.name());

  if (!fun || funcNode.isDecl()) {
    // Create function type double(double, double,...)
    // last arg flase means it's not a vararg function
    std::vector<Type *> doubles(funcNode.args().size(),
                                Type::getDoubleTy(llvmContext_));
    FunctionType *ft = FunctionType::get(Type::getDoubleTy(llvmContext_),
                                         std::move(doubles), false);

    // Create function of type ft and insert it into theModule_ llvm module
    fun = Function::Create(ft, Function::ExternalLinkage, funcNode.name(),
                           theModule_.get());

    // Give each arg of Function fun a name
    unsigned i = 0;
    for (auto &arg : fun->args()) {
      arg.setName(funcNode.args()[i++]);
    }

  } else {
    // TODO: check if function signature matches with prototype declared before
  }

  lastFn_ = fun;
  if (funcNode.isDecl()) {
    return;
  }

  if (!fun->empty()) {
    return logError("function cannot be redefined");
  }

  // Create a basic block and add it at the end of Function fun.
  BasicBlock *bb = BasicBlock::Create(llvmContext_, "entry", fun);

  // Tell builder to insert new instructions into this new BB
  builder_.SetInsertPoint(bb);

  symTable_.clear();
  for (auto &arg : fun->args()) {
    symTable_[std::string(arg.getName())] = &arg;
  }

  // Generate code for function body
  funcNode.body()->accept(*this);
  if (!valStack_.empty()) {
    // Everything went well, generate ret instruction
    // returning function body expression value
    builder_.CreateRet(valStack_.front());
    valStack_.pop_front();
    verifyFunction(*fun);

    // Optimize the function
    theFPM_->run(*fun);
    return;
  }

  // Error in generating body, remove function
  fun->eraseFromParent();
  lastFn_ = nullptr;
}

void Codegen::printIR(const char *msg) const {
  std::cerr << msg << std::endl;
  if (lastFn_) {
    lastFn_->print(errs());
  }
  std::cerr << std::endl;
}

void Codegen::printModule() const { theModule_->print(errs(), nullptr); }

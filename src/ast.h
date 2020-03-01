#ifndef AST_H
#define AST_H

#include "visitor.h"
#include <cassert>
#include <memory>
#include <string>
#include <vector>

// Abstract base class for all nodes
class BaseNode {
public:
  using UPtr = std::unique_ptr<BaseNode>;
  virtual ~BaseNode() = default;
  virtual void accept(Visitor &visitor) = 0;
};

// Base class for all expression nodes
class ExprNode : public BaseNode {
public:
  using UPtr = std::unique_ptr<ExprNode>;
  void accept(Visitor &visitor) override { visitor.visit(*this); }
};

class NumberExprNode : public ExprNode {
public:
  NumberExprNode(double num) : num_(num) {}

  double num() const { return num_; }

  void accept(Visitor &visitor) override { visitor.visit(*this); }

private:
  double num_;
};

class VariableExprNode : public ExprNode {
public:
  VariableExprNode(std::string varName) : varName_(std::move(varName)) {}

  std::string varName() const { return varName_; }

  void accept(Visitor &visitor) override { visitor.visit(*this); }

private:
  std::string varName_;
};

class BinaryExprNode : public ExprNode {
public:
  enum Op {
    plus,
    minus,
    mul,
    div,
    mod,
  };

  BinaryExprNode(Op op, ExprNode::UPtr lhs, ExprNode::UPtr rhs)
      : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
  BinaryExprNode(int opc, ExprNode::UPtr lhs, ExprNode::UPtr rhs)
      : op_(plus), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {
    switch (opc) {
    case '+':
      op_ = plus;
      break;
    case '-':
      op_ = minus;
      break;
    case '*':
      op_ = mul;
      break;
    case '/':
      op_ = div;
      break;
    case '%':
      op_ = mod;
      break;
    default:
      assert(false && "invalid binary op");
      break;
    }
  }

  Op op() const { return op_; }
  ExprNode *lhs() const { return lhs_.get(); }
  ExprNode *rhs() const { return rhs_.get(); }

  void accept(Visitor &visitor) override { visitor.visit(*this); }

private:
  Op op_;
  ExprNode::UPtr lhs_;
  ExprNode::UPtr rhs_;
};

class CallExprNode : public ExprNode {
public:
  CallExprNode(std::string callee, std::vector<ExprNode::UPtr> args)
      : callee_(std::move(callee)), args_(std::move(args)) {}

  std::string callee() const { return callee_; }
  const std::vector<ExprNode::UPtr> &args() const { return args_; }

  void accept(Visitor &visitor) override { visitor.visit(*this); }

private:
  std::string callee_;
  std::vector<ExprNode::UPtr> args_;
};

class IfElseExprNode : public ExprNode {
public:
  IfElseExprNode(ExprNode::UPtr condExpr, ExprNode::UPtr thenExpr,
                 ExprNode::UPtr elseExpr)
      : condExpr_(std::move(condExpr)), thenExpr_(std::move(thenExpr)),
        elseExpr_(std::move(elseExpr)) {}

  ExprNode *condExpr() const { return condExpr_.get(); }
  ExprNode *thenExpr() const { return thenExpr_.get(); }
  ExprNode *elseExpr() const { return elseExpr_.get(); }

  void accept(Visitor &visitor) override { visitor.visit(*this); }

private:
  ExprNode::UPtr condExpr_;
  ExprNode::UPtr thenExpr_;
  ExprNode::UPtr elseExpr_;
};

class FunctionNode : public BaseNode {
public:
  FunctionNode(bool isDecl, std::string name, std::vector<std::string> args,
               ExprNode::UPtr body)
      : isDecl_(isDecl), name_(std::move(name)), args_(std::move(args)),
        body_(std::move(body)) {}

  using UPtr = std::unique_ptr<FunctionNode>;

  bool isDecl() const { return isDecl_; }
  std::string name() const { return name_; }
  const std::vector<std::string> &args() const { return args_; }
  ExprNode *body() const { return body_.get(); }

  void accept(Visitor &visitor) override { visitor.visit(*this); }

private:
  bool isDecl_;
  std::string name_;
  std::vector<std::string> args_;
  ExprNode::UPtr body_;
};

#endif // AST_H

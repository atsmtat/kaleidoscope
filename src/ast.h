#ifndef AST_H
#define AST_H

#include <cassert>
#include <memory>
#include <string>
#include <vector>

// Base class for all expression nodes
class ExprNode {
public:
  using UPtr = std::unique_ptr<ExprNode>;
  virtual ~ExprNode() = default;
};

class NumberExprNode : public ExprNode {
public:
  NumberExprNode(double num) : num_(num) {}

  double num() const { return num_; }

private:
  double num_;
};

class VariableExprNode : public ExprNode {
public:
  VariableExprNode(std::string varName) : varName_(std::move(varName)) {}

  std::string varName() const { return varName_; }

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
    switch( opc ) {
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
      assert( false && "invalid binary op" );
      break;
    }
  }

  Op op() const { return op_; }
  ExprNode * lhs() const { return lhs_.get(); }
  ExprNode * rhs() const { return rhs_.get(); }

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

private:
  std::string callee_;
  std::vector<ExprNode::UPtr> args_;
};

class FunctionNode {
public:
 FunctionNode(bool isDecl, std::string name, std::vector<std::string> args,
               ExprNode::UPtr body)
   : isDecl_(isDecl), name_(std::move(name)),
    args_(std::move(args)), body_(std::move(body)) {}

  using UPtr = std::unique_ptr< FunctionNode >;

  bool isDecl() const { return isDecl_; }
  std::string name() const { return name_; }
  const std::vector<std::string> & args() const { return args_; }
  ExprNode * body() const { return body_.get(); }

private:
  bool isDecl_;
  std::string name_;
  std::vector<std::string> args_;
  ExprNode::UPtr body_;
};

#endif // AST_H

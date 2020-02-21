#pragma once

class ExprNode;
class NumberExprNode;
class VariableExprNode;
class BinaryExprNode;
class CallExprNode;
class FunctionNode;

class Visitor {
public:
  virtual void visit( ExprNode & ) = 0;
  virtual void visit( NumberExprNode & ) = 0;
  virtual void visit( VariableExprNode & ) = 0;
  virtual void visit( BinaryExprNode & ) = 0;
  virtual void visit( CallExprNode & ) = 0;
  virtual void visit( FunctionNode & ) = 0;
};

#ifndef PARSER_H
#define PARSER_H

#include <unordered_map>

#include "ast.h"
#include "lexer.h"

class Parser {
 public:
  Parser( Lexer & lexer ) : currToken_( Token::EOF_TOK ), lexer_( lexer ) {
    initializeBinOpPrecedence();
  }
  void parse();

 private:
  void initializeBinOpPrecedence();
  
  int getNextToken() {
    currToken_ = lexer_.getToken();
    return currToken_;
  }

  int currToken() const {
    return currToken_;
  }

  double currNum() const {
    return lexer_.numberVal();
  }

  std::string currIdentifier() const {
    return lexer_.identifierStr();
  }

  int getTokPrecedence();
  void logError( const char * msg );
  ExprNode::UPtr parseNumberExpr();
  ExprNode::UPtr parseParenExpr();
  ExprNode::UPtr parseIdentExpr();
  ExprNode::UPtr parsePrimary();
  ExprNode::UPtr parseBinOpRHS( int minPrec, ExprNode::UPtr lhs );
  ExprNode::UPtr parseExpr();

  FunctionNode::UPtr parseFunction();
  FunctionNode::UPtr parseLambdaExpr();

  void handleFunction();
  void handleLambdaExpr();
  
  int currToken_;
  Lexer & lexer_;
  std::unordered_map< BinaryExprNode::Op, int > binOpPrecedence_;
};

#endif // PARSER_H

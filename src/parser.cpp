#include <iostream>

#include "parser.h"

/*
numberexpr -> NUMBER
identexpr -> IDENT
           | IDENT '(' ( (expression ',')* expression )? ')'
parenexpr -> '(' expression ')'
function -> def IDENT '(' IDENT* ')' expression
          | extern IDENT '(' IDENT* ')'

 */

void
Parser::initializeBinOpPrecedence() {
  binOpPrecedence_[ BinaryExprNode::Op::minus ] = 10;
  binOpPrecedence_[ BinaryExprNode::Op::plus ] = 20;
  binOpPrecedence_[ BinaryExprNode::Op::mul ] = 30;
  binOpPrecedence_[ BinaryExprNode::Op::div ] = 40;
  binOpPrecedence_[ BinaryExprNode::Op::mod ] = 50; // highest
}

int
Parser::getTokPrecedence() {
  switch( currToken_ ) {
  case '+':
    return binOpPrecedence_[ BinaryExprNode::Op::plus ];
    break;
  case '-':
    return binOpPrecedence_[ BinaryExprNode::Op::minus ];
    break;
  case '*':
    return binOpPrecedence_[ BinaryExprNode::Op::mul ];
    break;
  case '/':
    return binOpPrecedence_[ BinaryExprNode::Op::div ];
    break;
  case '%':
    return binOpPrecedence_[ BinaryExprNode::Op::mod ];
    break;
  default:
    return -1;
    break;
  }
}

void
Parser::logError( const char * msg ) {
  std::cerr << "error: " << msg << std::endl;
  return;
}

ExprNode::UPtr
Parser::parseNumberExpr() {
  auto numExpr = std::make_unique< NumberExprNode >( currNum() );

  // consume NUMBER
  getNextToken();
  return std::move( numExpr );
}

ExprNode::UPtr
Parser::parseParenExpr() {
  // consume '('
  getNextToken();

  auto expr = parseExpr();
  if( !expr ) {
    return nullptr;
  }

  if( currToken() != ')' ) {
    logError( "expected ')'" );
    return nullptr;
  }

  // consume ')'
  getNextToken();
  return std::move( expr );
}

ExprNode::UPtr
Parser::parseIdentExpr() {
  std::string identStr = currIdentifier();

  // consume IDENT
  getNextToken();

  if( currToken() != '(' ) {
    return std::make_unique< VariableExprNode >( identStr );
  }

  // consume '('
  getNextToken();

  std::vector< ExprNode::UPtr > args;
  if( currToken() != ')' ) {
    while( true ) {
      auto arg = parseExpr();
      if( arg ) {
	args.push_back( std::move( arg ) );
      } else {
	return nullptr;
      }

      if( currToken() == ')' ) {
	break;
      }

      if( currToken() != ',' ) {
	logError( "expected ')' or ',' in argument list" );
	return nullptr;
      }

      // consume ','
      getNextToken();
    }
  }

  // consume ')'
  getNextToken();

  return std::make_unique< CallExprNode >( identStr, std::move( args ) );
}

ExprNode::UPtr
Parser::parsePrimary() {
  switch( currToken() ) {
  case NUMBER:
    return parseNumberExpr();
    break;
  case IDENT:
    return parseIdentExpr();
    break;
  case '(':
    return parseParenExpr();
    break;
  default:
    logError( "unknown token while parsing expression" );
    return nullptr;
    break;
  }
}

ExprNode::UPtr
Parser::parseBinOpRHS( int minPrec, ExprNode::UPtr lhs ) {
  while( true ) {
    int currPrec = getTokPrecedence();

    if( currPrec < minPrec ) {
      return lhs;
    }

    int currBinOp = currToken();
    // consume bin op
    getNextToken();

    auto rhs = parsePrimary();
    if( !rhs ) {
      return nullptr;
    }

    int nextPrec = getTokPrecedence();
    if( currPrec < nextPrec ) {
      rhs = parseBinOpRHS( currPrec + 1, std::move( rhs ) );
      if( !rhs ) {
	return nullptr;
      }
    }

    lhs = std::make_unique< BinaryExprNode >( currBinOp, std::move( lhs ), std::move( rhs ) );
  }
}

ExprNode::UPtr
Parser::parseExpr() {
  auto lhs = parsePrimary();
  if( !lhs ) {
    return nullptr;
  }

  return parseBinOpRHS( 0, std::move( lhs ) );
}

FunctionNode::UPtr
Parser::parseFunction() {
  bool isDecl = currToken() == Token::EXTERN;

  // consume 'extern' or 'def'
  getNextToken();

  if( currToken() != Token::IDENT ) {
    logError( "expected function name" );
    return nullptr;
  }

  std::string funcName = currIdentifier();
  // consume IDENT
  getNextToken();

  if( currToken() != '(' ) {
    logError( "expected '(' in function declaration" );
    return nullptr;
  }

  std::vector< std::string > args;
  while( getNextToken() == IDENT ) {
    args.push_back( currIdentifier() );
  }

  if( currToken() != ')' ) {
    logError( "expected ')' in function declaration" );
    return nullptr;
  }

  // consume ')'
  getNextToken();

  ExprNode::UPtr funcBody = nullptr;
  if( !isDecl ) {
    // function def should have a body
    funcBody = parseExpr();
    if( !funcBody ) {
      return nullptr;
    }
  }
  return std::make_unique< FunctionNode >( isDecl, funcName, std::move( args ), std::move( funcBody ) );
}

FunctionNode::UPtr
Parser::parseLambdaExpr() {
  if( auto expr = parseExpr() ) {
    return std::make_unique< FunctionNode >( false, "", std::vector<std::string>{}, std::move( expr ) );
  }
  return nullptr;
}

void
Parser::handleFunction() {
  if( parseFunction() ) {
    std::cerr << "Parsed a function" << std::endl;
  } else {
    // consume token for error recovery
    getNextToken();
  }
}

void
Parser::handleLambdaExpr() {
  if( parseLambdaExpr() ) {
    std::cerr << "Parsed a lamba expression" << std::endl;
  } else {
    // consume token for error recovery
    getNextToken();
  }
}

void
Parser::parse() {
  getNextToken();
  while( true ) {
    std::cout << "kscope>";
    switch( currToken() ) {
    case EOF_TOK:
      return;
    case ';':
      getNextToken();
      break;
    case DEF:
      handleFunction();
      break;
    case EXTERN:
      handleFunction();
      break;
    default:
      handleLambdaExpr();
      break;
    }
  }
}
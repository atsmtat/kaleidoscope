#include <limits>

#include "lexer.h"

int
Lexer::getToken() {
  while( isspace( currChar_ ) ) {
    currChar_ = in_.get();
  }

  if( isalpha( currChar_ ) ) {
    identifierStr_ = currChar_;
    currChar_ = in_.get();
    while( isalnum( currChar_ ) ) {
      identifierStr_.push_back( currChar_ );
      currChar_ = in_.get();
    }

    if( identifierStr_ == "def" ) {
      return DEF;
    }
    if( identifierStr_ == "extern" ) {
      return EXTERN;
    }
    return IDENT;
  }

  if( isdigit( currChar_ ) || currChar_ == '.' ) {
    std::string numStr;
    do {
      numStr.push_back( currChar_ );
      currChar_ = in_.get();
    } while( isdigit( currChar_ ) || currChar_ == '.' );
    numberVal_ = stod( numStr );
    return NUMBER;
  }

  if( currChar_ == '#' ) {
    in_.ignore( std::numeric_limits< std::streamsize >::max(), '\n' );
    if( in_.eof() ) {
      return EOF_TOK;
    }
    return getToken();
  }

  if( in_.eof() ) {
    return EOF_TOK;
  }
  char tok = currChar_;
  currChar_ = in_.get();
  return tok;
}

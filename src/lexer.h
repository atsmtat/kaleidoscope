#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>

enum Token {
  EOF_TOK = -1,

  DEF = -2,
  EXTERN = -3,

  IDENT = -4,
  NUMBER = -5
};

class Lexer {
 public:
 Lexer( const std::istream & input ) : currChar_( ' ' ), in_( input.rdbuf() ) {}
  int getToken();

  std::string identifierStr() const { return identifierStr_; }
  double numberVal() const { return numberVal_; }

 private:
  char currChar_;
  std::string identifierStr_;
  double numberVal_;
  std::istream in_;
};

#endif // LEXER_H

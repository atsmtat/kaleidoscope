#include <iostream>

#include "lexer.h"
#include "parser.h"

int
main() {
  Lexer lex{ std::cin };
  Parser parser{ lex };

  std::cout << "kscope>";
  parser.parse();

  return 0;
}

#include <iostream>

#include "parser.hpp"

int main() {

  try {
    Parser prs("test0.txt");
    prs.run();
    Tree::print(prs.tree->root);
  } catch (const std::exception &e) {
    std::cerr << "Compile error: " << e.what() << '\n';
  }

  return 0;
}
/* g++ -std=c++20  -o test main.cpp ..\..\src\parser\parser.cpp
 * ..\..\src\parser\parser_final.cpp ..\..\src\ast\tree.cpp
 * ..\..\src\lexer\tokenizer.cpp*/
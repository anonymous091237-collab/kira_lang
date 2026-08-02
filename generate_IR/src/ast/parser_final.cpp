#include "parser.hpp"
#include <iostream>


Parser::Parser(const string &path) {
  tree = new Tree();
  tokenizer = new Tokenizer(path);
  tokenizer->run();
  tokens = &tokenizer->tokens;
}

Parser::~Parser() {
  delete tree;
  delete tokenizer;
}

void Parser::peek() {
  if (it >= tokens->size()) {
    text = std::string_view{};
    return;
  }

  currentToken = &(*tokens)[it];
  tkType = static_cast<TokenType>(
      *((char *)tokenizer->tokensAdr + currentToken->id));

  text = std::string_view((char *)tokenizer->tokensAdr + currentToken->id + 1);
}
// this one just returns a string of a text of smt
std::string_view Parser::peek(int offset) {
  if ((it + offset) >= tokens->size()) {
    return std::string_view{};
  }
  return {(char *)tokenizer->tokensAdr + ((*tokens)[it + offset]).id + 1};
}

void Parser::move(int offset) {
  it += offset;
  peek();
}

bool isNumber(std::string *num) {
  for (auto &c : *num) {
    if (c < '0' || c > '9') {
      return false;
    }
  }
  return true;
}

static bool alpha_(const char &c) {
  return ((c == '_') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'));
}

static bool alphanum_(const char &c) {
  return ((c == '_') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
          ('0' <= c && c <= '9'));
}

bool isIdentifier(std::string *text) {
  if (!alpha_((*text)[0]))
    return false;
  for (auto &c : *text) {
    if (!alphanum_(c))
      return false;
  }
  return true;
}

void Parser::errorLog(const std::string &error) {
  if (currentToken) {
    std::string s(currentToken->pos.chr - 1, ' ');
    std::cout << currentToken->pos.chr << ':' << currentToken->pos.line << '\n';
    throw std::runtime_error(
        error + "\n" +
        tokenizer->fileContent.substr(currentToken->pos.index -
                                          currentToken->pos.chr,
                                      currentToken->pos.chr) +
        '\n' + s + '^');
  } else {
    throw std::runtime_error(error + "\ntoken empty!\n");
  }
}

void Parser::run() {
  peek();
  tree->root = ParseSourceCode();
  std::cout << "AST finished without errors!\n";
}

Node Parser::ParseSourceCode() {
  Node code = Tree::createNode(NodeType::NONE, "Code");
  int i = 1;
  Node *child;
  while (it < tokens->size()) {
    child = Tree::getChild(code, i);
    (*child) = parseFunDeclaration();
    i++;
  }
  return code;
}

Node Parser::parseFunctionCall() {
  Node callNode = Tree::createNode(NodeType::CALL, text);
  move(2); // consume identifier and '('

  int it = 1;
  while (!text.empty() && text != ")") {
    Node *argument = Tree::getChild(callNode, it);

    *argument = Tree::createNode(NodeType::ARG);
    (*argument)->child = parseExpression();

    if (text == ",")
      move();

    it++;
  }
  if (text != ")")
    errorLog("Expected ')' after function call");

  move();

  return callNode;
}

Node Parser::parseBlock() {
  if (text.empty() || (text != "{"))
    errorLog("expected '{'");

  move(); // consume '{'
  Node block = Tree::createNode(NodeType::BLOCK);

  if (!text.empty() && (text == "}")) {
    move(); // move out of the block
    return block;
  }

  int it = 1;

  while (!text.empty() && text != "}") {
    Node *instruction = Tree::getChild(block, it);

    if (text == "def" || text == "var") {
      *instruction = parseVarDeclaration();
    } else if (text == "return") {
      *instruction = parseReturn();
    } else if (text == "if") {
      *instruction = ParseIfstatement();
    } else if (text == "while") {
      *instruction = ParseWhilestatement();
    } else if (tkType == TokenType::Identifier) {
      *instruction = parseExpression(); // parseIdentifiers();
    }

    else {
      errorLog("unknown command!");
    }
    if (peek(-1) == "}") {
      it++;
      continue;
    }
    if (text.empty() || text != ";") {
      errorLog("expected ';' after command end!");
    }
    move();
    it++;
  }

  if (text.empty() || text != "}") {
    throw std::runtime_error("Expected '}' at end of block!\n");
  }
  move(); // exit block

  return block;
}

Node Parser::parseIdentifiers() {
  // identifier is a function call
  if (peek(1) == "(") {
    return parseFunctionCall();
  } else {
    Node n = Tree::createNode(NodeType::ID, text);

    move(); // consume identifier

    return n;
  }
}

Node Parser::parseMultiplication() {
  // parse primary expression first!
  Node left = parseUnary();

  while (!text.empty() && (text == "*" || text == "/" || text == "%")) {
    Node parent = Tree::createNode(NodeType::BIOP, text);

    move(); // consume operator
    Node right = parseUnary();
    parent->child = left;
    left->sibling = right;

    left = parent;
  }

  return left;
}

Node Parser::parseAddition() {
  // parse multiplication first!
  Node left = parseMultiplication();
  // then parse the addition
  while (!text.empty() && (text == "+" || text == "-")) {
    Node parent = Tree::createNode(NodeType::BIOP, text);

    move(); // consume the operator
    Node right = parseMultiplication();

    parent->child = left;
    left->sibling = right;

    left = parent;
  }

  return left;
}

Node Parser::parseComparison() {
  Node left = parseAddition();

  while (!text.empty() &&
         (text == "<" || text == ">" || text == "<=" || text == ">=")) {
    Node parent = Tree::createNode(NodeType::BIOP, text);
    move();

    Node right = parseAddition();

    parent->child = left;
    left->sibling = right;

    left = parent;
  }

  return left;
}

Node Parser::parseEquality() {
  Node left = parseComparison();

  while (!text.empty() && (text == "==" || text == "!=")) {
    Node parent = Tree::createNode(NodeType::BIOP, text);
    move();

    Node right = parseComparison();

    parent->child = left;
    left->sibling = right;

    left = parent;
  }

  return left;
}

Node Parser::parseLogicalAnd() {
  Node left = parseEquality();

  while (!text.empty() && (text == "&&" || text == "and")) {
    Node parent = Tree::createNode(NodeType::BIOP, text);
    move();

    Node right = parseEquality();

    parent->child = left;
    left->sibling = right;

    left = parent;
  }

  return left;
}

Node Parser::parseLogicalOr() {
  Node left = parseLogicalAnd();

  while (!text.empty() && (text == "||" || text == "or")) {
    Node parent = Tree::createNode(NodeType::BIOP, text);
    move();

    Node right = parseLogicalAnd();

    parent->child = left;
    left->sibling = right;

    left = parent;
  }

  return left;
}

Node Parser::parseAssign() {
  Node left = parseLogicalOr();

  if (!text.empty() && (text == "=" || text == "+=" || text == "-=" ||
                        text == "*=" || text == "/=" || text == "%=")) {
    Node parent = Tree::createNode(NodeType::ASSIGN, text);

    move();

    Node right = parseAssign();

    parent->child = left;
    left->sibling = right;

    return parent;
  }

  return left;
}
Node Parser::parseExpression() { return parseAssign(); }
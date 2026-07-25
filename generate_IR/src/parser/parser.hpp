#pragma once
#include "../ast/tree.hpp"
#include "../lexer/tokenizer.hpp"

using std::string;
using std::vector;

bool isNumber(std::string* num);
bool isIdentifier(std::string* num);

class Parser {
   private:
    void peek();
    std::string_view peek(int offset);
    void move(int offset = 1);

    Tokenizer* tokenizer = nullptr;
    vector<Token>* tokens = nullptr;
    Token* currentToken = nullptr;

    std::string_view text;
    TokenType tkType = TokenType::None;

    uint32_t it = 0;

    void errorLog(const std::string& error = "");

    Node parsePrimary();
    Node parseFunctionCall();
    Node parseUnary();
    Node parseMultiplication();
    Node parseAddition();
    Node parseComparison();
    Node parseEquality();
    Node parseLogicalAnd();
    Node parseLogicalOr();
    Node parseAssign();
    Node parseExpression();

    Node parseIdentifiers();
    Node parseReturn();
    Node parseVarDeclaration();

    Node parseBlock();
    Node ParseWhilestatement();
    Node ParseIfstatement();
    Node parseParams_helper();
    Node parseParams();
    Node parseFunDeclaration();
    Node ParseSourceCode();

   public:
    Tree* tree = nullptr;
    void run();

    Parser(const string& path);
    ~Parser();
};
#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../ast/tree.hpp"
#include "binary_ir.hpp"

class Converter {
   private:
    int number = 0;
    std::vector<Reusable> item;
    std::vector<uint8_t> IR_array;
    // string result = "";

    bool found_main = false;

    std::unordered_map<string, VarId> my_variables;
    std::unordered_map<string, VarId> my_functions;
    void varDeclaration(const Node& variable);
    void varExpression(const Node& node, size_t src = 0, size_t base_idx = 0);
    void printFunction(const Node& command);
    void functionCall(const Node& command);
    void whileStatement(const Node& statement);
    void ifStatement(const Node& statement);
    void returnStatement(const Node& command);
    void GenerateInstructions(const Node& Block);

    void GenerateFunction(const Node& start);
    void code();
    const string IRfilename;

   public:
    bool saveAST = false;
    VarId mainId;
    Node AST = nullptr;
    string output;

    Converter(Node root, const std::string& irFilename);
    void run();
};

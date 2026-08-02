#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "binary_ir.hpp"
#include "tree.hpp"

class Converter {
private:
  Reusable gen_operand(const Node &node);

  bool genOperator(const Node &node, size_t src, size_t base_idx);

  void std_print(const Node &node);

  Reusable gen_assign(const Node &node);
  Reusable gen_unop(const Node &node);
  Reusable gen_biop(const Node &node);
  Reusable gen_call(const Node &node);
  Reusable gen_expression(const Node &node);

  Id createTemp();
  Id get_id(string_view input);
  Id get_fn(string_view input);

  int idCounter = 0;
  int fnCounter = 0;

  int number = 0;

  std::vector<uint8_t> IR_array;

  bool found_main = false;

  std::unordered_map<string_view, Id> my_variables;
  std::unordered_map<string_view, Id> my_functions;
  void varDeclaration(const Node &variable);

  void printFunction(const Node &command);

  void whileStatement(const Node &statement);
  void ifStatement(const Node &statement);
  void returnStatement(const Node &command);
  void GenerateInstructions(const Node &Block);

  void GenerateFunction(const Node &start);
  void code();
  const string IRfilename;

public:
  std::string buildPath;
  bool saveAST = false;
  Id mainId;
  Node AST = nullptr;
  string output;

  Converter(Node root, std::string irFilename)
      : IRfilename(std::move(irFilename)), AST(root) {}
  void run();
};

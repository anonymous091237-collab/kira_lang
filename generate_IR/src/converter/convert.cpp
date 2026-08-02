#include "convert.hpp"
#include "binary_ir.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

Type str_to_type(const std::string_view &src) {

  auto idx = typeHashMap.find(src);
  if (idx == typeHashMap.end()) {
    throw std::runtime_error("unvalide binary operator!");
  }
  return idx->second;
}

OP str_to_unop(const std::string_view &src) {
  if (src == "-") {
    return OP::NEG;
  }
  throw std::runtime_error("unvalide unary operator!");
}

OP str_to_biop(const std::string_view &src) {

  auto idx = biopHashMap.find(src);
  if (idx == biopHashMap.end()) {
    throw std::runtime_error("unvalide binary operator!");
  }
  return idx->second;
}

Id Converter::createTemp() { return static_cast<Id>(idCounter++); }

Id Converter::get_id(string_view input) {

  const auto it = my_variables.find(input);
  if (it == my_variables.end()) {

    const auto result = static_cast<Id>(idCounter++);
    my_variables[input] = result;
    return result;
  }
  return it->second;
}

Id Converter::get_fn(string_view input) {

  const auto it = my_functions.find(input);
  if (it == my_functions.end()) {

    const auto result = static_cast<Id>(fnCounter++);
    my_functions[input] = result;
    return result;
  }
  return it->second;
}

Reusable Converter::gen_operand(const Node &node) {
  Reusable result;
  if (node->type == NodeType::ID) {
    result.set_var(get_id(node->name));
  } else if (node->type == NodeType::NUM) {
    result.set_num(node->name);
  } else if (node->type == NodeType::STR) {
    result.set_str(string_view{node->name.data() + 1, node->name.size() - 2});
  }
  return result;
}

Reusable Converter::gen_biop(const Node &node) {
  struct PendingOp {
    OP op;
    Node right;
  };
  std::vector<PendingOp> ops;

  auto *current = node;
  while (current->type == NodeType::BIOP) {
    ops.push_back({.op = str_to_biop(current->name),
                   .right = *Tree::getChild(current, 2)});
    current = *Tree::getChild(current, 1);
  }

  Reusable acc = gen_expression(current);

  for (auto it = ops.rbegin(); it != ops.rend(); ++it) {
    Reusable right = gen_expression(it->right);
    Id temp = createTemp();
    append_op(IR_array, it->op);
    acc.write_to(IR_array);
    right.write_to(IR_array);
    append_id(IR_array, temp);
    acc.set_var(temp);
  }
  return acc;
}

Reusable Converter::gen_unop(const Node &node) {
  auto *const expression = *Tree::getChild(node, 1);
  const auto operation = str_to_unop(node->name);
  const Id temp = createTemp();

  Reusable operand = gen_expression(expression);
  append_op(IR_array, operation);
  operand.write_to(IR_array);
  append_id(IR_array, temp);

  Reusable result;
  result.set_var(temp);
  return result;
}

Reusable Converter::gen_assign(const Node &node) {
  auto *const leftExpr = *Tree::getChild(node, 1);
  auto *const rightExpr = *Tree::getChild(node, 2);

  Reusable left = gen_expression(leftExpr);
  Reusable right = gen_expression(rightExpr);

  if (node->name[0] == '=') {
    append_op(IR_array, OP::STORE);
    right.write_to(IR_array);
    left.write_to(IR_array);
    return left;
  }

  // compound assignment: += -= etc.
  const auto operation = str_to_biop({node->name.data(), 1});
  const Id temp = createTemp();

  append_op(IR_array, operation);
  left.write_to(IR_array);
  right.write_to(IR_array);
  append_id(IR_array, temp);

  append_op(IR_array, OP::STORE);
  append_id(IR_array, temp);
  left.write_to(IR_array);

  return left;
}

void Converter::std_print(const Node &node) {
  Node arg = node->child;
  while (arg != nullptr) {
    append_op(IR_array, OP::CALL);
    append_op(IR_array, OP::STD);
    append_std(IR_array, std_lib::PRINT);

    Reusable val = gen_expression(arg->child);
    val.write_to(IR_array);

    arg = arg->sibling;
  }
}

Reusable Converter::gen_call(const Node &node) {
  const auto &functionName = node->name;
  const uint8_t arg_count = Tree::countChild(node);

  if (const auto it = std_functions.find(functionName);
      it != std_functions.end()) {
    if (it->second == std_lib::PRINT) {
      std_print(node);
    }
    return Reusable{};
  }

  if (const auto it = my_functions.find(functionName);
      it != my_functions.end()) {

    // evaluate all args first
    std::vector<Reusable> args;
    args.reserve(arg_count);
    for (int i = 0; i < arg_count; i++) {
      Node arg = *Tree::getChild(*Tree::getChild(node, i + 1), 1);
      args.push_back(gen_expression(arg));
    }

    append_op(IR_array, OP::CALL);
    append_op(IR_array, OP::USR);
    append_id(IR_array, it->second);
    IR_array.push_back(arg_count);
    for (auto &arg : args) {
      arg.write_to(IR_array);
    }

    const Id result = createTemp();
    Reusable out;
    out.set_var(result);
    append_id(IR_array, result);
    return out;
  }

  throw std::runtime_error("unknown function!");
}

Reusable Converter::gen_expression(const Node &node) {
  if (node->type == NodeType::ID || node->type == NodeType::NUM ||
      node->type == NodeType::STR) {
    return gen_operand(node);
  }
  switch (node->type) {
  case NodeType::UNOP:
    return gen_unop(node);
  case NodeType::ASSIGN:
    return gen_assign(node);
  case NodeType::BIOP:
    return gen_biop(node);
  case NodeType::CALL:
    return gen_call(node);
  default:
    throw std::runtime_error("unknown expression!");
  }
}

void Converter::GenerateFunction(const Node &function) {
  string_view function_name = (*Tree::getChild(function, 1))->name;

  Id function_id = get_id(function_name);

  if (!found_main && function_name == "main") {
    found_main = true;
    mainId = function_id;
  }

  string_view function_type = (*Tree::getChild(function, 3))->name;

  append_op(IR_array, OP::FUN);
  append_id(IR_array, function_id);
  append_type(IR_array, str_to_type(function_type));
  // FUN <function_id> <function_type>

  Node parameters = (*Tree::getChild(function, 2));

  int param_count = Tree::countChild(parameters);
  int it = 0;
  while (it < param_count) {
    Node param = (*Tree::getChild(parameters, it + 1));
    string_view param_name = (*Tree::getChild(param, 1))->name;
    string_view param_type = (*Tree::getChild(param, 2))->name;

    Id new_varname = get_id(param_name);

    append_op(IR_array, OP::PARAM);
    // PARAM

    if (param->is_constant) {
      append_op(IR_array, OP::DEF);
      // DEF
    } else {
      append_op(IR_array, OP::VAR);
      // VAR
    }
    append_id(IR_array, new_varname);
    append_type(IR_array, str_to_type(param_type));
    // <new_varname> <param_type>
    it++;
  }

  GenerateInstructions(*Tree::getChild(function, 4));

  append_op(IR_array, OP::END);
}

void Converter::returnStatement(const Node &command) {
  if (command->child != nullptr) {
    Reusable value = gen_expression(command->child);
    append_op(IR_array, OP::RET);
    value.write_to(IR_array);
  } else {
    append_op(IR_array, OP::RET);
    append_tag(IR_array, Tag::VOID);
  }
}

void Converter::varDeclaration(const Node &variable) {
  string_view varname = (*Tree::getChild(variable, 1))->name;
  string_view vartype = (*Tree::getChild(variable, 2))->name;
  Id new_varname = get_id(varname);

  if (variable->is_constant) {
    Reusable value = gen_expression(*Tree::getChild(variable, 3));
    append_op(IR_array, OP::DEF);
    append_id(IR_array, new_varname);
    append_type(IR_array, str_to_type(vartype));
    value.write_to(IR_array);
  } else {
    append_op(IR_array, OP::VAR);
    append_id(IR_array, new_varname);
    append_type(IR_array, str_to_type(vartype));

    if (Tree::countChild(variable) >= 3) {
      Reusable value = gen_expression(*Tree::getChild(variable, 3));
      append_op(IR_array, OP::STORE);
      value.write_to(IR_array);
      append_id(IR_array, new_varname);
    }
  }
}

void Converter::whileStatement(const Node &statement) {
  Id startCondition = createTemp();
  Id startLoop = createTemp();
  Id endLoop = createTemp();

  append_op(IR_array, OP::JMP);
  append_id(IR_array, startCondition);
  append_op(IR_array, OP::LABEL);
  append_id(IR_array, startCondition);

  Reusable condition = gen_expression(statement->child);
  append_op(IR_array, OP::JMPC);
  condition.write_to(IR_array);
  append_id(IR_array, startLoop);
  append_op(IR_array, OP::JMP);
  append_id(IR_array, endLoop);

  append_op(IR_array, OP::LABEL);
  append_id(IR_array, startLoop);
  GenerateInstructions(*Tree::getChild(statement, 2));
  append_op(IR_array, OP::JMP);
  append_id(IR_array, startCondition);

  append_op(IR_array, OP::LABEL);
  append_id(IR_array, endLoop);
}

void Converter::ifStatement(const Node &statement) {
  Reusable condition = gen_expression(statement->child);
  Id labelA = createTemp();
  Id labelB = createTemp();
  Id labelC = createTemp();

  append_op(IR_array, OP::JMPC);
  condition.write_to(IR_array);
  append_id(IR_array, labelA);
  append_op(IR_array, OP::JMP);
  append_id(IR_array, labelB);
  append_op(IR_array, OP::LABEL);
  append_id(IR_array, labelA);

  GenerateInstructions(*Tree::getChild(statement, 2));

  append_op(IR_array, OP::JMP);
  append_id(IR_array, labelC);
  append_op(IR_array, OP::LABEL);
  append_id(IR_array, labelB);
  append_op(IR_array, OP::JMP);
  append_id(IR_array, labelC);
  append_op(IR_array, OP::LABEL);
  append_id(IR_array, labelC);
}

void Converter::GenerateInstructions(const Node &Block) {
  int cmd_count = Tree::countChild(Block);
  for (int it = 0; it < cmd_count; it++) {
    Node cmd = *Tree::getChild(Block, it + 1);
    switch (cmd->type) {
    case NodeType::VARDEC:
      varDeclaration(cmd);
      break;
    case NodeType::CALL:
      gen_call(cmd);
      break;
    case NodeType::RETSTMNT:
      returnStatement(cmd);
      break;
    case NodeType::IFSTMNT:
      ifStatement(cmd);
      break;
    case NodeType::WHILESTMNT:
      whileStatement(cmd);
      break;
    default:
      gen_expression(cmd);
      break;
    }
  }
}

static void add_function(std::unordered_map<string_view, Id> &functions,
                         const string_view &src, int &fnCounter) {
  const auto it = functions.find(src);
  if (it != functions.end()) {
    throw std::runtime_error("this function already exists! " + string(src));
  }
  functions[src] = static_cast<Id>(fnCounter++);
}

/**
 * ## Main Program Logic
 */
void Converter::code() {

  const auto function_count = Tree::countChild(AST);

  int it = 0;

  while (it < function_count) {
    Node function = (*Tree::getChild(AST, it + 1));

    add_function(my_functions, function->child->name, fnCounter);

    GenerateFunction(function);
    it++;
  }
}

void save_to_file(const std::string &path, const std::vector<uint8_t> &array) {
  std::ofstream file(path, std::ios::binary);

  if (!file)
    throw std::runtime_error("Failed to open file: " + path);

  file.write(reinterpret_cast<const char *>(array.data()),
             static_cast<std::streamsize>(array.size()));
}

void Converter::run() {
  std::cout << "-generating IR...\n";

  code();
  append_id(IR_array, mainId);

  std::cout << "-IR generated successfully!\n";
  std::cout << "-saving IR to file...\n";

  const auto outPath =
      std::filesystem::path(buildPath) / (IRfilename + "IR.bin");

  save_to_file(outPath.string(), IR_array);
}

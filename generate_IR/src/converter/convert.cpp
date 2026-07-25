#include "convert.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include "../../../include/constants.hpp"
#include "../../../include/debug.h"
template <typename... Args>
void save_to_str(string& res, const Args&... args) {
    (res += ... += args);
}

int idCounter = 0;

VarId createTemp() { return static_cast<VarId>(idCounter++); }

Converter::Converter(Node root, const std::string& irFilename)
    : IRfilename(irFilename) {
    AST = root;
}

VarId get_id(auto& table, string_view input) {
    auto [it, inserted] = table.try_emplace(std::string(input), createTemp());

    return it->second;
}

void Converter::varExpression(const Node& node, size_t src, size_t base_idx) {
    if (base_idx + 1 > item.size()) {
        size_t requierd = item.size() == 0 ? 2 : item.size();
        while (base_idx + 1 > requierd) {
            requierd *= 2;
        }
        item.resize(requierd);
    }
    if (node->type == NodeType::ID) {
        item[src].set_var(get_id(my_variables, node->name));
        return;
    }
    if (node->type == NodeType::NUM) {
        item[src].set_num(node->name);
        return;
    }
    if (node->type == NodeType::STR) {
        item[src].set_str(
            string_view{node->name.data() + 1, node->name.size() - 2});
        return;
    }
    if (node->type == NodeType::UNOP) {
        varExpression(*Tree::getChild(node, 1), base_idx, base_idx + 1);
        // Reusable* right = &item[base_idx];
        VarId temp = createTemp();

        // example : NEG right temp

        append_op(IR_array, str_to_unop.at(node->name));
        item[base_idx].write_to(IR_array);
        append_var(IR_array, temp);

        item[src].set_var(temp);  // temp is a variable
        return;
    }
    if (node->type == NodeType::ASSIGN) {  // regular assignment

        if (node->name == "=") {
            varExpression(*Tree::getChild(node, 1), base_idx, base_idx + 2);
            varExpression(*Tree::getChild(node, 2), base_idx + 1, base_idx + 2);
            // Reusable* left = &item[base_idx];
            // Reusable* right = &item[base_idx + 1];
            // STORE right  left
            append_op(IR_array, OP::STORE);
            item[base_idx + 1].write_to(IR_array);
            item[base_idx].write_to(IR_array);
            item[src] = item[base_idx];  // return left; (left type is unknown)
            return;

        } else {  // conpound assignment : += -= etc...

            varExpression(*Tree::getChild(node, 1), base_idx, base_idx + 2);
            varExpression(*Tree::getChild(node, 2), base_idx + 1, base_idx + 2);
            // Reusable* left = &item[base_idx];
            // Reusable* right = &item[base_idx + 1];

            string_view firstchar{node->name.data(), 1};
            // std::cout << "first char " << firstchar << '\n';
            VarId temp = createTemp();

            append_op(IR_array, str_to_biop.at(firstchar));
            item[base_idx].write_to(IR_array);
            item[base_idx + 1].write_to(IR_array);
            append_var(IR_array, temp);
            // ADD/MUL... left right temp

            append_op(IR_array, OP::STORE);
            append_var(IR_array, temp);
            item[base_idx].write_to(IR_array);
            // STORE temp left
            item[src] = item[base_idx];  // return left; (left type is unknown)
            return;
        }
    }
    if (node->type == NodeType::BIOP) {
        Node cur = node;

        struct PendingOp {
            string_view op;
            Node right;
        };
        std::vector<PendingOp> ops;

        // collect all ops and right children going left
        while (cur->type == NodeType::BIOP) {
            Node right = *Tree::getChild(cur, 2);
            ops.push_back({cur->name, right});
            cur = *Tree::getChild(cur, 1);
        }

        // cur is now the leftmost leaf
        varExpression(cur, base_idx, base_idx + 2);

        // emit in reverse (left to right)
        for (int i = ops.size() - 1; i >= 0; i--) {
            varExpression(ops[i].right, base_idx + 1, base_idx + 2);

            VarId temp = createTemp();
            append_op(IR_array, str_to_biop.at(ops[i].op));

            item[base_idx].write_to(IR_array);
            item[base_idx + 1].write_to(IR_array);
            append_var(IR_array, temp);
            item[src].set_var(temp);
            item[base_idx].set_var(temp);
        }
        return;
    }
    if (node->type == NodeType::CALL) {
        VarId temp = createTemp();
        /*Reusable* arg = &item[0];
        arg->set_var(temp);*/
        functionCall(node /* ,*arg*/);

        // return temp;
        item[src].set_var(temp);
        return;
    }

    return;
}
void Converter::varDeclaration(const Node& variable) {
    string_view varname = (*Tree::getChild(variable, 1))->name;
    string_view vartype = (*Tree::getChild(variable, 2))->name;

    VarId new_varname = get_id(my_variables, varname);

    if (variable->is_constant) {
        Node* default_value = Tree::getChild(variable, 3);

        varExpression(*default_value, 0);
        Reusable* value = &item[0];
        append_op(IR_array, OP::DEF);
        append_var(IR_array, new_varname);
        append_type(IR_array, str_to_Type.at(vartype));
        value->write_to(IR_array);

        // DEF <new_varname> <vartype> <value>

    } else {
        // VAR <new_varname> <vartype>
        append_op(IR_array, OP::VAR);
        append_var(IR_array, new_varname);
        append_type(IR_array, str_to_Type.at(vartype));

        if (Tree::countChild(variable) >= 3) {
            Node* initial_value = Tree::getChild(variable, 3);

            varExpression(*initial_value, 0);
            Reusable* value = &item[0];
            append_op(IR_array, OP::STORE);
            value->write_to(IR_array);
            append_var(IR_array, new_varname);

            // STORE <value> <new_varname>
        }
    }
}

void Converter::whileStatement(const Node& statement) {
    VarId startCondition = createTemp();
    VarId startLoop = createTemp();
    VarId endLoop = createTemp();

    append_op(IR_array, OP::JMP);
    append_var(IR_array, startCondition);
    // JMP <startCondition>
    append_op(IR_array, OP::LABEL);
    append_var(IR_array, startCondition);
    // LABEL <startCondition>

    varExpression(statement->child, 0);
    Reusable* condition = &item[0];
    append_op(IR_array, OP::JMPC);
    condition->write_to(IR_array);
    append_var(IR_array, startLoop);
    // JMPC <condition> <startLoop>
    append_op(IR_array, OP::JMP);
    append_var(IR_array, endLoop);
    // JMP <endLoop>

    append_op(IR_array, OP::LABEL);
    append_var(IR_array, startLoop);
    // LABEL <startLoop>

    Node* block = Tree::getChild(statement, 2);
    GenerateInstructions(*block);

    append_op(IR_array, OP::JMP);
    append_var(IR_array, startCondition);
    // JMP <startCondition>
    append_op(IR_array, OP::LABEL);
    append_var(IR_array, endLoop);
    // LABEL <endLoop>
}

void Converter::ifStatement(const Node& statement) {
    varExpression(statement->child, 0);
    Reusable* condition = &item[0];
    VarId labelA = createTemp();
    VarId labelB = createTemp();
    VarId labelC = createTemp();

    append_op(IR_array, OP::JMPC);
    condition->write_to(IR_array);
    append_var(IR_array, labelA);
    append_op(IR_array, OP::JMP);
    append_var(IR_array, labelB);
    append_op(IR_array, OP::LABEL);
    append_var(IR_array, labelA);
    // JMPC <condition> <labelA> JMP <labelB> LABEL <labelA>

    Node* instructions = Tree::getChild(statement, 2);
    GenerateInstructions(*instructions);

    append_op(IR_array, OP::JMP);
    append_var(IR_array, labelC);
    append_op(IR_array, OP::LABEL);
    append_var(IR_array, labelB);

    append_op(IR_array, OP::JMP);
    append_var(IR_array, labelC);
    append_op(IR_array, OP::LABEL);
    append_var(IR_array, labelC);

    // JMP <labelC> LABEL <labelB> JMP <labelC> LABEL <labelC>
}

void Converter::GenerateFunction(const Node& function) {
    string_view function_name = (*Tree::getChild(function, 1))->name;

    VarId function_id = get_id(my_functions, function_name);

    if (!found_main && function_name == "main") {
        found_main = true;
        mainId = function_id;
    }

    string_view function_type = (*Tree::getChild(function, 3))->name;

    append_op(IR_array, OP::FUN);
    append_var(IR_array, function_id);
    append_type(IR_array, str_to_Type.at(function_type));
    // FUN <function_id> <function_type>

    Node parameters = (*Tree::getChild(function, 2));

    int param_count = Tree::countChild(parameters);
    int it = 0;
    while (it < param_count) {
        Node param = (*Tree::getChild(parameters, it + 1));
        string_view param_name = (*Tree::getChild(param, 1))->name;
        string_view param_type = (*Tree::getChild(param, 2))->name;

        VarId new_varname = get_id(my_variables, param_name);

        append_op(IR_array, OP::PARAM);
        // PARAM

        if (param->is_constant) {
            append_op(IR_array, OP::DEF);
            // DEF

        } else {
            append_op(IR_array, OP::VAR);
            // VAR
        }
        append_var(IR_array, new_varname);
        append_type(IR_array, str_to_Type.at(param_type));
        // <new_varname> <param_type>
        it++;
    }

    GenerateInstructions(*Tree::getChild(function, 4));

    append_op(IR_array, OP::END);
}

void Converter::printFunction(const Node& command) {
    int arg_count = Tree::countChild(command);
    int it = 0;
    Node arg;
    DEBUG_PRINT("printFunction arg_count: %d\n", arg_count);

    while (it < arg_count) {
        arg = (*Tree::getChild(command, it + 1))->child;
        // CALL STD PRINT
        append_op(IR_array, OP::CALL);
        append_op(IR_array, OP::STD);
        append_std(IR_array, std_lib::PRINT);

        if (arg->type == NodeType::ID) {
            VarId var = get_id(my_variables, arg->name);
            append_var(IR_array, var);
            // var
        }
        if (arg->type == NodeType::NUM) {
            append_num(IR_array, arg->name);
        }
        if (arg->type == NodeType::STR) {
            DEBUG_PRINT(
                "printed string %s\n",
                string_view(arg->name.data() + 1, arg->name.size() - 2));
            /*std::cout << "printed string "
                      << string_view(arg->name.data() + 1, arg->name.size() - 2)
                      << '\n';*/
            append_str(IR_array,
                       string_view(arg->name.data() + 1, arg->name.size() - 2));
        }

        it++;
    }
}

void Converter::functionCall(const Node& command /*, const Reusable& arg*/) {
    if (command->name == "print") {
        printFunction(command);
    }
}

void Converter::returnStatement(const Node& command) {
    if (command->child != nullptr) {
        Node return_value = command->child;

        varExpression(return_value, 0);
        Reusable* value = &item[0];
        append_op(IR_array, OP::RET);
        value->write_to(IR_array);
        // save_to_str(result, "RET ", ret_result, '\n');
    } else {
        append_op(IR_array, OP::RET);

        append_tag(IR_array, Tag::VOID);
        // save_to_str(result, "RET VOID\n");
    }
}

void Converter::GenerateInstructions(const Node& Block) {
    int cmd_count = Tree::countChild(Block);

    Node cmd;
    int it = 0;
    while (it < cmd_count) {
        cmd = *Tree::getChild(Block, it + 1);
        NodeType cmdtyp = (cmd)->type;
        switch (cmdtyp) {
            case NodeType::VARDEC:
                varDeclaration(cmd);
                break;
            case NodeType::CALL:
                functionCall(cmd);
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

                varExpression(cmd);
        }
        it++;
    }
}

void Converter::code() {
    int function_count = Tree::countChild(AST);
    int it = 0;
    while (it < function_count) {
        Node function = (*Tree::getChild(AST, it + 1));
        GenerateFunction(function);
        it++;
    }
}
/*




*/

void ast_save_helper(std::ofstream& file, const Node father, int depth) {
    Node p = father->child;

    while (p != nullptr) {
        for (int i = 0; i < depth; i++) file << "    ";
        std::string_view nodetype = NodeTypeNames[static_cast<size_t>(p->type)];
        if (!p->name.empty()) {
            file << '<' << nodetype << "> " << p->name << '\n';
        } else if (nodetype == "Block") {
            file << "<Block>\n";
        } else {
            file << '<' << nodetype << ">\n";
        }

        ast_save_helper(file, p, depth + 1);
        p = p->sibling;
    }
}
void ast_save(std::ofstream& file, const Node father) {
    std::string_view nodetype =
        NodeTypeNames[static_cast<size_t>(father->type)];
    if (!father->name.empty()) {
        file << '<' << nodetype << "> " << father->name << '\n';
    } else if (nodetype == "Block") {
        file << "<Block>\n";
    } else {
        file << '<' << nodetype << ">\n";
    }

    ast_save_helper(file, father, 1);
}
/*





*/
void save_to_file(const string& path, const Node& root) {
    std::ofstream file(path);

    if (!file.is_open()) {
        std::cerr << "Unable to open file";
    }
    ast_save(file, root);
    file.close();
}

void save_to_file(const std::string& path, const std::vector<uint8_t>& array) {
    std::ofstream file(path, std::ios::binary);

    if (!file) throw std::runtime_error("Failed to open file: " + path);

    file.write(reinterpret_cast<const char*>(array.data()),
               static_cast<std::streamsize>(array.size()));
}
#include <filesystem>
void Converter::run() {
    std::cout << "-generating IR...\n";
    item.resize(4);
    // result.reserve(16 * 1024);

    code();
    append_var(IR_array, mainId);

    std::cout << "-IR generated sucessfully!\n";
    std::cout << "-saving IR to file...\n";
    // save_to_file(".\\k_build\\" + IRfilename + "IR.bin", IR_array);

    std::filesystem::path inputPath(IRfilename);
    const auto dir = inputPath.parent_path();
    const auto baseName = inputPath.stem().string();
    const auto outPath =

        (!dir.empty() ? dir : std::filesystem::path(".\\k_build")) /
        (baseName + "IR.bin");

    save_to_file(outPath.string(), IR_array);

    if (saveAST) {
        std::cout << "-saving ast to file...\n";
        save_to_file(".\\k_build\\ast.txt", AST);
        std::cout << "-AST successfully saved!\n";
    }
}


#include "compiler.hpp"

#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "../../include/debug.h"
#include "navigateIR.hpp"

template <typename... Args>
void save_to_str(string& res, const Args&... args) {
    (res += ... += args);
#ifdef _DEBUG
    std::cout << "written llvm :";
    (std::cout << ... << args) << '\n';
#endif
}
bool is_type_int(const Type& v) { return (Type::i8 <= v && v <= Type::u64); }
void LlvmBuilder::generate_function() {
    const OP startop = nextOp(current_token);
    if (current_token >= last_token || startop != OP::FUN) {
        throw std::runtime_error(
            "expected operator 'FUN' marking start function!");
    }
    const Tag tag = nextTag(current_token);
    if (tag != Tag::ID) {
        throw std::runtime_error("expected tag 'ID' marking function id");
    }
    const VarId id = nextId(current_token);

    FunInfo f = functions.at(id);
    if (id == mainId) {
        result += std::format("define {} @main(", Type_to_str.at(f.type));
    } else {
        result += std::format("define {} @fn{}(", Type_to_str.at(f.type), f.id);
    }
    skipType(current_token);

    OP op = nextOp(current_token);
    while (op == OP::PARAM) {
        const OP keyword = nextOp(current_token);
        if (keyword != OP::DEF) {
            throw std::runtime_error(
                "expected keyword DEF at function parameter!");
        }
        const Tag param_tag = nextTag(current_token);
        if (param_tag != Tag::ID) {
            throw std::runtime_error(
                "expected expected ID tag before parameter id!");
        }
        const VarId param_id = nextId(current_token);

        Type param_type = nextType(current_token);

        result += std::format("{} %v{},", Type_to_str.at(param_type), param_id);
        op = nextOp(current_token);
    }

    if (f.param_count != 0) result.pop_back();  // get rid of the last ','

    result += std::format(") {{\nentry:\n");

    functions[f.id] = f;

    while (current_token < last_token && op != OP::END) {
        DEBUG_PRINT("entering generate instructions...\n");
        generate_instuctions(f.id, op);
    }

    if (op != OP::END) {
        throw std::runtime_error(
            "expected operator 'END' to mark function ending!");
    }
    result += "}\n";
}

void LlvmBuilder::generate_instuctions(VarId function_name, OP& op) {
    if (current_token >= last_token || !isOP(op)) {
        throw std::runtime_error("expected operator!");
    }

    if (op == OP::VAR) {
        DEBUG_PRINT("op : VAR\n");
        gen_var();

    } else if (op == OP::DEF) {
        DEBUG_PRINT("op : DEF\n");
        gen_const();

    } else if (op == OP::STORE) {
        DEBUG_PRINT("op : STORE\n");
        gen_store();

    } else if (OP::ADD <= op && op <= OP::GE) {  // binary operations
        DEBUG_PRINT("op : binary operation ADD->GE\n");
        gen_math(op);
    } else if (op == OP::JMPC) {
        DEBUG_PRINT("op : JMPC\n");
        skipTag(current_token);
        const VarId condition = nextId(current_token);
        skipTag(current_token);
        const VarId label1 = nextId(current_token);

        result += std::format("br i1 %v{}, label %l{}, ", condition, label1);

        op = nextOp(current_token);

        if (op != OP::JMP) {
            throw std::runtime_error("expected operator JMP after JMPC!");
        }
        skipTag(current_token);
        const VarId label2 = nextId(current_token);
        result += std::format("label %l{}\n", label2);

    } else if (op == OP::JMP) {
        DEBUG_PRINT("op : JMP\n");
        skipTag(current_token);
        const VarId label = nextId(current_token);
        result += std::format("br label %l{}\n", label);

    } else if (op == OP::LABEL) {
        DEBUG_PRINT("op : LABEL\n");
        skipTag(current_token);
        const VarId label = nextId(current_token);
        result += std::format("l{}:\n", label);

    } else if (op == OP::NEG) {
        DEBUG_PRINT("op : NEG\n");
        gen_negative();
    } else if (op == OP::CALL) {
        DEBUG_PRINT("op : CALL\n");
        op = nextOp(current_token);
        if (op == OP::STD) {
            DEBUG_PRINT("calling std\n");
            standard_functions();
        }
    } else if (op == OP::RET) {
        DEBUG_PRINT("op : RET\n");
        const Tag tag = nextTag(current_token);
        if (Tag::NUM_I8 <= tag && tag <= Tag::NUM_I64) {
            const uint64_t num_value = nextNum(current_token, tag);

            result += std::format(
                "ret {} {}\n", Type_to_str.at(functions.at(function_name).type),
                num_value);
        } else {
            throw std::runtime_error("currently only supports numeric return!");
        }
    }
    op = nextOp(current_token);
    if (op == OP::END) {
        DEBUG_PRINT("op : END\n");
        return;
    }
}
void LlvmBuilder::standard_functions() {
    DEBUG_PRINT("entering standard_functions()\n");
    std_lib function = nextSTD(current_token);
    if (function == std_lib::PRINT) {
        print_function();
    }
}
void LlvmBuilder::gen_negative() {
    const Tag tagA = nextTag(current_token);
    bool A_isvar = (tagA == Tag::ID);

    uint64_t a =
        (A_isvar) ? nextId(current_token) : nextNum(current_token, tagA);
    skipTag(current_token);
    const VarInfo dst = variables.at(nextId(current_token));

    if (A_isvar) {
        VarInfo vA = variables.at(a);
        if (vA.is_pointer) {
            a = load_ptr(result, vA);
        }
    }

    result +=
        std::format("%v{} = sub {} 0, ", dst.id, Type_to_str.at(dst.type));
    if (A_isvar) result += "%v";

    result += std::format("{}\n", a);
}

void allocate_var(string& result, const VarInfo& v) {
    // "%v{} = alloca {}, align {}\n", v.id, Type_to_str.at(v.type),
    // get_alignment(v.type)
    result += std::format("%v{} = alloca {}, align {}\n", v.id,
                          Type_to_str.at(v.type), get_alignment(v.type));
}

void LlvmBuilder::gen_var() {
    const Tag varTag = nextTag(current_token);
    if (varTag != Tag::ID) {
        throw std::runtime_error(
            "expected expected ID tag before var declaration!");
    }

    VarInfo v = variables.at(nextId(current_token));
    skipType(current_token);

    allocate_var(result, v);
}

void LlvmBuilder::gen_const() {
    const Tag varTag = nextTag(current_token);
    if (varTag != Tag::ID) {
        throw std::runtime_error(
            "expected expected ID tag before const declaration!");
    }
    VarInfo v = variables.at(nextId(current_token));

    skipType(current_token);

    result += std::format("%v{}", v.id);
    const Tag valueTag = nextTag(current_token);

    if (is_type_int(v.type)) {
        result += std::format(" = add {} ", Type_to_str.at(v.type));

        if (valueTag == Tag::ID) {  // it's a variable not a litteral
            result += std::format("%v{}, 0\n", nextId(current_token));

        } else if (Tag::NUM_I8 <= valueTag && valueTag <= Tag::NUM_I64) {
            result += std::format("{}, 0\n", nextNum(current_token, valueTag));
        } else {
            throw std::runtime_error(
                "currently consts can only supports numeric types!");
        }
    }
}

void LlvmBuilder::gen_store() {
    const Tag valueTag = nextTag(current_token);

    // string_view value = next();

    // VarInfo dst = variables.at(next());

    if (valueTag == Tag::ID) {
        const VarId src = nextId(current_token);

        const VarInfo v_src = variables.at(src);

        skipTag(current_token);
        const VarInfo res = variables.at(nextId(current_token));

        if (v_src.is_pointer) {  // v_src is a mutable var that has to be loaded
                                 // first

            const VarId src_value = load_ptr(result, v_src);
            result += std::format("store {} %v{}", Type_to_str.at(res.type),
                                  src_value);
        } else {
            result += std::format("store {} %v{}", Type_to_str.at(res.type),
                                  v_src.id);
        }
        result += std::format(", ptr %v{}, align {}\n", res.id,
                              get_alignment(res.type));

    } else {
        const VarId value = nextNum(current_token, valueTag);
        skipTag(current_token);
        const VarInfo res = variables.at(nextId(current_token));
        result += std::format("store {} {}, ptr %v{} , align {}\n",
                              Type_to_str.at(res.type), value, res.id,
                              get_alignment(res.type));
    }
    // store i32 458 ,ptr
    /*if (res.is_pointer) { RES IS A POINTER BY DEFAULT YOU CANNOT STORE ON
    CONSTS result += std::format( ", ptr %", res.name, ", align ",
                    get_alignment(res.type), '\n');
    } */
}
char get_alignment(const Type& type) {
    if (type == Type::i16 || type == Type::u16) return '2';

    if (type == Type::i32 || type == Type::u32) return '4';

    if (type == Type::i64 || type == Type::u64) return '8';

    return '1';  // for char and bool
}

VarId load_ptr(string& output, const VarInfo& v) {
    const VarId dst = create_temp();

    // "%v{} = load {} , ptr %v{}, align {}\n",dst,Type_to_str.at(v.type),
    // v.id,get_alignment(v.type)
    output += std::format("%v{} = load {} , ptr %v{}, align {}\n", dst,
                          Type_to_str.at(v.type), v.id, get_alignment(v.type));

    return dst;
}
VarId create_temp() {
    tmpnum++;
    return tmpnum + var_count;
}

void LlvmBuilder::gen_math(const OP& oprtr) {
    const Tag leftTag = nextTag(current_token);

    uint64_t left = (leftTag == Tag::ID) ? nextId(current_token)
                                         : nextNum(current_token, leftTag);

    const Tag rightTag = nextTag(current_token);

    uint64_t right = (rightTag == Tag::ID) ? nextId(current_token)
                                           : nextNum(current_token, rightTag);

    skipType(current_token);
    const VarInfo dst = variables.at(nextId(current_token));

    gen_math_expression(result, oprtr, left, right, leftTag, rightTag, dst,
                        variables);
}

void gen_math_expression(string& result, const OP& oprtr, uint64_t& A,
                         uint64_t& B, const Tag tagA, const Tag tagB,
                         const VarInfo& dst, varTable& variables) {
    string_view math_op;
    switch (oprtr) {
        case OP::ADD:
            math_op = "add";
            break;
        case OP::SUB:
            math_op = "sub";
            break;
        case OP::MUL:
            math_op = "mul";
            break;
        case OP::DIV:
            math_op = "sdiv";
            break;
        case OP::MOD:
            math_op = "srem";
            break;
        default:
            break;
    }

    Type cmp_type = Type::i32;

    bool A_isvar = (tagA == Tag::ID);
    bool B_isvar = (tagB == Tag::ID);

    if (A_isvar) {
        const VarInfo vA = variables.at(A);

        cmp_type = vA.type;
        if (vA.is_pointer) {
            A = load_ptr(result, vA);
        }
    }
    if (B_isvar) {
        const VarInfo vB = variables.at(B);
        cmp_type = vB.type;
        if (vB.is_pointer) {
            B = load_ptr(result, vB);
        }
    }

    // //"%quotient = sdiv i32 %a, %b"

    if (!dst.is_pointer) {
        result += std::format("%v{} = ", dst.id);
    }

    if (oprtr > OP::MOD) {
        // temporarly treat every comparison as int
        result += std::format("icmp {}", llvm_cmp(oprtr, is_signed(cmp_type)));
    } else {
        result += math_op;
    }

    result += ' ';

    if (oprtr > OP::MOD) {
        result += Type_to_str.at(cmp_type);
    } else {
        result += Type_to_str.at(dst.type);
    }
    result += ' ';

    if (A_isvar) {
        result += "%v";
    }
    result += std::format("{}, ", A);

    if (B_isvar) {
        result += "%v";
    }
    result += std::format("{}\n", B);
}
bool is_signed(const Type& type) {
    if (type == Type::i8 || type == Type::i16 || type == Type::i32 ||
        type == Type::i64)
        return true;

    return false;
}

string_view llvm_cmp(const OP& op, bool isSigned) {
    switch (op) {
        case OP::EQ:
            return "eq";
        case OP::NE:
            return "ne";
        case OP::GT:
            return isSigned ? "sgt" : "ugt";
        case OP::GE:
            return isSigned ? "sge" : "uge";
        case OP::LT:
            return isSigned ? "slt" : "ult";
        case OP::LE:
            return isSigned ? "sle" : "ule";
        default:
            return "";
    }
}

bool is_numeric(string_view sv) {
    if (sv.empty()) return false;

    if (sv[0] == '-') sv.remove_prefix(1);
    for (char c : sv) {
        if ('0' > c || c > '9') return false;
    }
    return true;
}

void LlvmBuilder::generate_ll(const string& ll_path) {
    std::ofstream file(ll_path);

    file.write(result.data(), result.size());
    file.write("\n", 1);

    file.close();
}

void LlvmBuilder::load_IR(const string& IR_path) {
    std::ifstream file(IR_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + IR_path);
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    tokens.resize(size);

    if (!file.read(reinterpret_cast<char*>(tokens.data()), size)) {
        throw std::runtime_error("Failed to read file: " + IR_path);
    }

    mainId = static_cast<VarId>(tokens[tokens.size() - 4]) |
             (static_cast<VarId>(tokens[tokens.size() - 3]) << 8) |
             (static_cast<VarId>(tokens[tokens.size() - 2]) << 16) |
             (static_cast<VarId>(tokens[tokens.size() - 1]) << 24);
    tokens.resize(tokens.size() - 4);

    file.close();
}
void LlvmBuilder::run() {
    current_token = tokens.data();
    last_token = current_token + tokens.size();
    VarId maxId = 0;
    for (auto& [id, _] : variables) maxId = std::max(maxId, id);
    for (auto& [id, _] : functions) maxId = std::max(maxId, id);
    var_count = maxId;
    tmpnum = 0;
    printf("var_count %d\n", var_count);
    define_globals();
    // temporarly always include print
    // while (current_token < last_token) {
    generate_function();
    // }
    std::cout << "IR compiled to llvm sucessfully!\n";
}

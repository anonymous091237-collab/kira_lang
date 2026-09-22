#include "compile.hpp"
#include <iostream>
using StringEntry = std::pair<const std::string, string_info> *;

void bubbleSort(std::vector<StringEntry> &pool) {
    size_t n = pool.size();
    bool swapped;
    StringEntry temp;
    for (size_t i = 0; i < n - 1; ++i) {
        swapped = false;
        for (size_t j = 0; j < n - i - 1; ++j) {
            if (pool[j]->second.offset > pool[j + 1]->second.offset) {
                temp = pool[j];
                pool[j] = pool[j + 1];
                pool[j + 1] = temp;
                swapped = true;
            }
        }
        if (!swapped)
            break;
    }
}
void wrtie_llvm_str(StringEntry &src, std::string &dst, size_t &size) {
    size_t start = size;
    const char *data = src->first.data();
    size_t len = src->first.size();

    for (size_t i = 0; i < len; i++) {
        if (data[i] == '\\' && i + 1 < len) {
            i++; // consume the next char
            switch (data[i]) {
            case 'n':
                dst += "\\0A";
                break;
            case 't':
                dst += "\\09";
                break;
            default:
                // unknown escape, emit both chars as-is
                dst.push_back('\\');
                dst.push_back(data[i]);
                size++; // extra byte for the backslash
                break;
            }
        } else {
            dst.push_back(data[i]);
        }
        size++; // one logical byte per iteration (escape counts as 1)
    }
    src->second.size = size - start;
}
void compile::fill_str_array() {

    flat_str_array.clear();
    if (G.string_pool.empty()) {

        return;
    }
    std::vector<StringEntry> pool;
    pool.reserve(G.string_pool.size());

    for (auto &i : G.string_pool) {
        pool.push_back(&i);
    }
    bubbleSort(pool);

    for (auto &str : pool) {
        str->second.offset = flat_str_size;
        wrtie_llvm_str(str, flat_str_array, flat_str_size);
    }
    std::cout << "string pool :  \"" << flat_str_array << "\"\n";
}
#include "debug.hpp"

void write_op(OP op, std::ostream &out) {
    switch (op) {
    case OP::ADD: {
        out << "add";
        return;
    }
    case OP::MUL: {
        out << "mul";
        return;
    }
    case OP::SUB: {
        out << "sub";
        return;
    }
    case OP::DIV: {
        out << "sdiv";
        return;
    }
    case OP::MOD: {
        out << "srem";
        return;
    }
    case OP::EQ:
        out << "icmp eq";
        return;
    case OP::NE:
        out << "icmp ne";
        return;
    case OP::LT:
        out << "icmp slt";
        return;
    case OP::LE:
        out << "icmp sle";
        return;
    case OP::GT:
        out << "icmp sgt";
        return;
    case OP::GE:
        out << "icmp sge";
        return;
    }
}

Id compile::load_ptr(const variable *var) {
    counter++;
    out << "%loaded_v" << std::to_string(counter) << " = load "
        << to_string(var->type) << ", ptr %v" << std::to_string(var->name)
        << '\n';
    return counter;
}

Id compile::check_var(const variable *var) {
    if (!var->isConst) {
        return load_ptr(var);
    }
    return 0;
}

void compile::write_var(const variable *var, Id id) {
    if (var->isConst) {
        out << "%v" << var->name;
    } else {
        out << "%loaded_v" << id;
    }
}
uint64_t readNum(const Value &v) {
    uint64_t num = 0;
    for (uint32_t b = 0; b < v.size; b++)
        num |= static_cast<uint64_t>(*static_cast<const uint8_t *>(v.data) +
                                     b) // ← your bug: +b not [b]
               << (b * 8);
    // correct form:
    num = 0;
    const auto *bytes = static_cast<const uint8_t *>(v.data);
    for (uint32_t b = 0; b < v.size; b++)
        num |= static_cast<uint64_t>(bytes[b]) << (b * 8);
    return num;
};
void compile::cmp_cmp(const function &f, const instruction &i) {
    const OP op = static_cast<OP>(i.option);

    const auto &result = i.result;
    operand left(*this, f, i.args[0]);
    operand right(*this, f, i.args[1]);

    // result is always a fresh SSA value (v1, v2...), never an alloca
    out << "%v" << getId(result.data) << " = ";
    write_op(op, out);

    //
    if (left.isId) {
        out << ' ' << to_string(left.var->type) << ' ';
    } else if (right.isId) {
        out << ' ' << to_string(right.var->type) << ' ';
    } else {
        out << " i32 ";
    }
    //
    left.write(*this);
    out << ", ";
    right.write(*this);
    out << '\n';
}
void compile::cmp_math(const function &f, const instruction &i) {
    const OP op = static_cast<OP>(i.option);

    const auto &result = i.result;
    operand left(*this, f, i.args[0]);
    operand right(*this, f, i.args[1]);

    // result is always a fresh SSA value (v1, v2...), never an alloca
    out << "%v" << getId(result.data) << " = ";
    write_op(op, out);
    out << ' ' << to_string(f.local.at(getId(result.data)).type) << ' ';
    left.write(*this);
    out << ", ";
    right.write(*this);
    out << '\n';
}
void compile::cmp_ret(const function &f, const instruction &i) {

    operand result(*this, f, i.result);
    out << "ret " << to_string(f.type) << ' ';
    result.write(*this);
    out << '\n';
}
void compile::cmp_store(const function &f, const instruction &i) {

    const auto var = f.local.at(getId(i.result.data));
    operand val(*this, f, i.args[0]);
    out << "store " << to_string(var.type) << ' ';
    val.write(*this);
    out << ", ptr %v" << var.name << '\n';
}
void compile::cmp_def(const function &f, const instruction &i) {
    const auto var = f.local.at(getId(i.result.data));
    out << "%v" << var.name << " = add " << to_string(var.type) << ' '
        << readNum(i.args[0]) << " , 0\n";
}
void compile::cmp_var(const function &f, const instruction &i) {

    const auto var = f.local.at(getId(i.result.data));
    out << "%v" << var.name << " = alloca " << to_string(var.type) << '\n';
}
void compile::cmp_instruction(const function &f, const instruction &i,
                              size_t index) {
    if (i.head == Inst::IMATH) {
        cmp_math(f, i);
    } else if (i.head == Inst::ICMP) {
        cmp_cmp(f, i);
    } else if (i.head == Inst::MOVE) {
        if (i.option == static_cast<uint8_t>(OP::RET)) {
            cmp_ret(f, i);
        } else if (i.option == static_cast<uint8_t>(OP::LABEL)) {
            out << "label" << getId(i.result.data) << ":\n";
        } else if (i.option == static_cast<uint8_t>(OP::JMPC)) {

            // br i1 %condition, label %true_label, label %false_label
            out << "br i1 %v" << getId(i.args[0].data) << ", label %label"
                << getId(i.result.data) << ", label %label";

        } else if (i.option == static_cast<uint8_t>(OP::JMP)) {
            if (f.instructions[index - 1].option ==
                static_cast<uint8_t>(OP::JMPC)) {

                // if the previous instruction in JMPC
                out << getId(i.result.data) << '\n';
            } else {
                out << "br label %label" << getId(i.result.data) << '\n';
            }
        }
    } else if (i.head == Inst::STORE) {
        cmp_store(f, i);

    } else if (i.head == Inst::DEC) {
        if (i.option == static_cast<uint8_t>(OP::DEF)) {
            cmp_def(f, i);
        } else {
            cmp_var(f, i);
        }
    } else if (i.head == Inst::UCALL) {
        cmp_call(f, i);
    } else if (i.head == Inst::SCALL) {
        if (i.option == static_cast<uint8_t>(Lib::PRINT)) {

            const auto str = i.args[0];
            if (str.type != Val_t::STR) {
                return;
            }
            const auto printed = G.string_pool.at(
                std::string{static_cast<const char *>(str.data), str.size});
            out << "call i64 @fwrite(ptr getelementptr inbounds (["
                << flat_str_size << " x i8], ptr @string_pool, i64 0, i64 "
                << printed.offset << "), i64 1, i64 " << printed.size
                << ", ptr %stdout)\n"; //
        }
    }
}
void compile::cmp_call(const function &f, const instruction &i) {
    // i.args[0] = function ID
    // i.args[1...] = actual arguments

    std::vector<operand> args;
    args.reserve(i.option);

    for (size_t n = 1; n < i.args.size(); ++n) {
        args.emplace_back(*this, f, i.args[n]);
    }

    const auto &result = f.local.at(getId(i.result.data));

    out << "%v" << getId(i.result.data) << " = call " << to_string(result.type)
        << " @fn_" << getId(i.args[0].data) << '(';

    for (size_t n = 0; n < args.size(); ++n) {
        if (n)
            out << ", ";

        if (args[n].isId) {
            out << to_string(args[n].var->type) << ' ';
        } else {
            out << "i32 ";
        }

        args[n].write(*this);
    }

    out << ")\n";
}
void compile::cmp_function(const function &f) {
    if (f.name == G.main_function) {
        out << "\ndefine " << to_string(f.type) << " @main(";

    } else {
        out << "\ndefine " << to_string(f.type) << " @fn_"
            << std::to_string(f.name) << '(';
    }

    for (auto p = f.params.begin(); p != f.params.end(); p++) {

        out << to_string(p->type) << " %v" << std::to_string(p->name);
        if (p < f.params.end() - 1) {
            out << ',';
        }
    }

    out << ") {\n";

    out << "%stdout = call ptr @__acrt_iob_func(i32 1)\n";

    for (size_t index = 0; index < f.instructions.size(); ++index) {
        const auto &i = f.instructions[index];

        cmp_instruction(f, i, index);
    }

    /*compile instruction
    here*/
    out << "}\n";
}

void compile::run() {

    std::cout << "============llvm file============\n";
    out << "@string_pool = private constant [" << flat_str_size << " x i8] c\""
        << flat_str_array << "\"\n"
        << "declare i64 @fwrite(ptr, i64, i64, ptr)\n"
           "declare ptr @__acrt_iob_func(i32)\n";
    for (auto &f : G.functions) {
        cmp_function(f);
    }
}
// clang --target=x86_64-w64-windows-gnu C:\dev\hello.ll -o C:\dev\hello.exe
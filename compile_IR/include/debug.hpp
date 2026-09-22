#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include "context.hpp"

// ---------------------------------------------------------------
// Forward helpers
// ---------------------------------------------------------------

inline std::string to_string(Val_t t) {
    switch (t) {
    case Val_t::ID:
        return "ID";
    case Val_t::NUM:
        return "NUM";
    case Val_t::STR:
        return "STR";
    case Val_t::PTR:
        return "PTR";
    case Val_t::NUL:
        return "NUL";
    }
    return "?";
}

inline std::string to_string(Inst h) {
    switch (h) {
    case Inst::IMATH:
        return "IMATH";
    case Inst::ICMP:
        return "ICMP";
    case Inst::UCALL:
        return "UCALL";
    case Inst::SCALL:
        return "SCALL";
    case Inst::DEC:
        return "DEC";
    case Inst::MOVE:
        return "MOV";
    case Inst::STORE:
        return "STORE";
    }
    return "?";
}

// Map OP byte back to a mnemonic (mirror your OP enum order)
inline std::string op_mnemonic(uint8_t op) {
    switch (static_cast<OP>(op)) {
    case OP::ADD:
        return "ADD";
    case OP::SUB:
        return "SUB";
    case OP::MUL:
        return "MUL";
    case OP::DIV:
        return "DIV";
    case OP::MOD:
        return "MOD";
    case OP::NEG:
        return "NEG";
    case OP::EQ:
        return "EQ";
    case OP::NE:
        return "NE";
    case OP::LT:
        return "LT";
    case OP::LE:
        return "LE";
    case OP::GT:
        return "GT";
    case OP::GE:
        return "GE";
    case OP::JMP:
        return "JMP";
    case OP::JMPC:
        return "JMPC";
    case OP::LABEL:
        return "LABEL";
    case OP::STORE:
        return "STORE";
    case OP::CALL:
        return "CALL";
    case OP::RET:
        return "RET";
    case OP::VAR:
        return "VAR";
    case OP::DEF:
        return "DEF";
    default:
        return "OP(" + std::to_string(op) + ")";
    }
}

inline std::string to_string(Type t) {
    switch (t) {
    case Type::unknown:
        return "?";
    case Type::i1:
        return "i1";
    case Type::i8:
        return "i8";
    case Type::i16:
        return "i16";
    case Type::i32:
        return "i32";
    case Type::i64:
        return "i64";
    case Type::u8:
        return "u8";
    case Type::u16:
        return "u16";
    case Type::u32:
        return "u32";
    case Type::u64:
        return "u64";
    case Type::ch:
        return "ch";
    case Type::str:
        return "str";
    }
    return "type(" + std::to_string(static_cast<int>(t)) + ")";
}

// ---------------------------------------------------------------
// Value printer
// ---------------------------------------------------------------

inline std::string print_value(const Value &v) {
    switch (v.type) {

    case Val_t::ID: {
        // Id is a uint32_t stored inline
        uint32_t id = 0;
        std::memcpy(&id, v.data, sizeof(id));
        return "v" + std::to_string(id);
    }

    case Val_t::NUM: {
        // Little-endian integer of v.size bytes
        uint64_t n = 0;
        std::memcpy(&n, v.data, v.size);
        return std::to_string(n);
    }

    case Val_t::STR: {
        std::string s(static_cast<const char *>(v.data), v.size);
        // Escape newlines / tabs for readability
        std::string out = "\"";
        for (char c : s) {
            if (c == '\n')
                out += "\\n";
            else if (c == '\t')
                out += "\\t";
            else if (c == '\r')
                out += "\\r";
            else
                out += c;
        }
        out += '"';
        return out;
    }

    case Val_t::PTR:
        return "ptr(" + std::to_string(reinterpret_cast<uintptr_t>(v.data)) +
               ")";

    case Val_t::NUL:
        return "<null>";
    }
    return "?";
}

// ---------------------------------------------------------------
// Instruction printer
// ---------------------------------------------------------------

inline void print_instruction(const instruction &inst, FILE *out = stdout) {
    std::string op_str;

    if (inst.head == Inst::SCALL) {
        // Option is a Lib enum (0 = PRINT, 1 = READ, etc.)
        switch (static_cast<Lib>(inst.option)) {
        case Lib::PRINT:
            op_str = "PRINT";
            break;
        case Lib::READ:
            op_str = "READ";
            break;
        case Lib::ALLOC:
            op_str = "ALLOC";
            break;
        case Lib::FREE:
            op_str = "FREE";
            break;
        default:
            op_str = "LIB(" + std::to_string(inst.option) + ")";
            break;
        }
    } else if (inst.head == Inst::UCALL) {
        // Option is arg_count
        op_str = "ARGS(" + std::to_string(inst.option) + ")";
    } else {
        op_str = op_mnemonic(inst.option);
    }

    fprintf(out, "  [%s | %s]", to_string(inst.head).c_str(), op_str.c_str());

    for (const auto &a : inst.args)
        fprintf(out, "  %s", print_value(a).c_str());

    if (inst.result.type != Val_t::NUL)
        fprintf(out, "  -> %s", print_value(inst.result).c_str());

    fprintf(out, "\n");
}
// ---------------------------------------------------------------
// Variable printer
// ---------------------------------------------------------------

inline void print_variable(const variable &v, FILE *out = stdout) {
    fprintf(out, "  %s %s : %s\n", v.isConst ? "def" : "var",
            std::to_string(v.name).c_str(), to_string(v.type).c_str());
}

// ---------------------------------------------------------------
// Function printer
// ---------------------------------------------------------------

inline void print_function(const function &fn, FILE *out = stdout) {
    fprintf(out, "┌─ fun %s : %s\n", std::to_string(fn.name).c_str(),
            to_string(fn.type).c_str());

    if (!fn.params.empty()) {
        fprintf(out, "│  params:\n");
        for (const auto &p : fn.params)
            print_variable(p, out);
    }

    if (!fn.local.empty()) {
        fprintf(out, "│  locals:\n");
        for (const auto &[id, v] : fn.local)
            print_variable(v, out);
    }

    fprintf(out, "│  body (%zu instructions):\n", fn.instructions.size());
    for (const auto &inst : fn.instructions)

    {
        print_instruction(inst, out);
    }

    fprintf(out, "└─ end %s\n\n", std::to_string(fn.name).c_str());
}
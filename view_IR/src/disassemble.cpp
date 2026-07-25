
#include <sstream>

#include "reader.hpp"

const char* op_name(OP op) {
    if (op > OP::END)
        return "???OP";
    else
        return StrOp[(size_t)op];
}

const char* Type_name(Type t) {
    switch (t) {
        case Type::i1:
            return "bool";
        case Type::i8:
            return "byte";
        case Type::i16:
            return "short";
        case Type::i32:
            return "int";
        case Type::i64:
            return "long";
        case Type::u8:
            return "ubyte";
        case Type::u16:
            return "ushort";
        case Type::u32:
            return "uint";
        case Type::u64:
            return "ulong";
        default:
            return "???Type";
    }
}

const char* std_name(std_lib s) {
    switch (s) {
        case std_lib::PRINT:
            return "PRINT";
        default:
            return "???std";
    }
}
std::ostream& pad(std::ostream& out, int indent) {
    for (int i = 0; i < indent * 4; ++i) out << ' ';
    return out;
}

void Reader::disassemble(std::ostream& out) {
    int indent = 0;

    while (!done()) {
        OP op = static_cast<OP>(next());

        if (op == OP::FUN) {
            const std::string fid = readVal();  // VAR-tagged function id
            Type ftyp = static_cast<Type>(next());
            if (indent > 0) out << '\n';  // blank line between functions
            pad(out, indent)
                << "FUN " << fid << " : " << Type_name(ftyp) << '\n';
            indent++;
            continue;
        }

        if (op == OP::END) {
            indent--;
            pad(out, indent) << "END\n\n";
            continue;
        }

        if (op == OP::PARAM) {
            pad(out, indent) << "PARAM ";
            OP kind = static_cast<OP>(next());
            out << (kind == OP::DEF ? "const " : "var ");
            const std::string vid = readVal();
            Type vtyp = static_cast<Type>(next());
            out << vid << " : " << Type_name(vtyp) << '\n';
            continue;
        }

        if (op == OP::DEF) {
            const std::string vid = readVal();
            Type vtyp = static_cast<Type>(next());
            const std::string val = readVal();  // ← add this
            pad(out, indent) << "DEF " << vid << " : " << Type_name(vtyp)
                             << " = " << val << '\n';
            continue;
        }
        if (op == OP::VAR) {
            const std::string vid = readVal();
            Type vtyp = static_cast<Type>(next());
            pad(out, indent) << "VAR " << vid << " : " << Type_name(vtyp);

            out << '\n';
            continue;
        }

        if (op == OP::STORE) {
            const std::string src = readVal();
            const std::string dst = readVal();
            pad(out, indent) << "STORE " << src << " -> " << dst << '\n';
            continue;
        }

        if (OP::ADD <= op && op <= OP::GE) {  // binary arithmetic / comparison
            const std::string lhs = readVal();
            const std::string rhs = readVal();
            const std::string dst = readVal();
            pad(out, indent) << op_name(op) << ' ' << lhs << ", " << rhs
                             << " -> " << dst << '\n';
            continue;
        }

        if (op == OP::NEG) {
            const std::string operand = readVal();
            const std::string dst = readVal();
            pad(out, indent) << "NEG " << operand << " -> " << dst << '\n';
            continue;
        }

        if (op == OP::RET) {
            pad(out, indent) << "RET";
            // optional return value
            if (!done() && peek() <= 5) {
                out << ' ' << readVal();
            }
            out << '\n';
            continue;
        }

        if (op == OP::JMP) {
            const std::string target = readVal();
            pad(out, indent) << "JMP " << target << '\n';
            continue;
        }
        if (op == OP::JMPC) {
            const std::string cond = readVal();
            const std::string target = readVal();
            pad(out, indent) << "JMPC " << cond << ", " << target << '\n';
            continue;
        }
        if (op == OP::LABEL) {
            const std::string lbl = readVal();
            // dedent label one level so it stands out
            int saved = indent;
            if (indent > 0) indent--;
            pad(out, indent) << "label " << lbl << ":\n";
            indent = saved;
            continue;
        }

        // ── call ────────────────────────────────────────────────────────
        if (op == OP::CALL) {
            uint8_t next_byte = next();

            if (static_cast<OP>(next_byte) == OP::STD) {
                // stdlib call: CALL STD <lib_id> <arg>
                const auto lib = static_cast<std_lib>(next());
                const std::string arg = readVal();
                pad(out, indent)
                    << "CALL STD." << std_name(lib) << ' ' << arg << '\n';
            } /*
            user function call

            else {


                // user function call: CALL <function_var>
                // next_byte is actually the Tag for the var
                // Back-track by one conceptually: we consumed the tag
                // already. Since next_byte must be VAR (0), reconstruct:
                Tag tag = static_cast<Tag>(next_byte);
                const std::string fid;
                if (tag == Tag::ID) {
                    auto id = r.read_var_id(r);
                    fid = "v" + std::to_string(id);
                } else {
                    fid = "???";
                }
                pad(out, indent) << "CALL " << fid << '\n';
            }*/
            continue;
        }

        // Unknown byte — print hex offset and value, keep going.
        out << "[UNKNOWN 0x" << std::hex
            << static_cast<int>(static_cast<uint8_t>(op)) << std::dec
            << " at offset " << (pos - 1) << "]\n";
        continue;
    }
}
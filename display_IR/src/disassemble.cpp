
// #include <sstream>
#include <ostream>
#include <stdexcept>

#include "reader.hpp"
// =============================================================
const char *op_name(OP op) {
    if (op > OP::USR)
        return "???OP";
    else
        return StrOp[(size_t)op];
}
// =============================================================
const char *Type_name(Type t) {
    if (t > Type::str)
        return "???Type";
    else
        return StrType[(size_t)t];
}
// =============================================================
const char *std_name(std_lib s) {
    switch (s) {
    case std_lib::PRINT:
        return "PRINT";
    default:
        return "???std";
    }
}

// =============================================================
void Reader::disassemble(std::ostream &out) {

    while (!done()) {

        const auto operation = static_cast<OP>(next());

        if (operation == OP::FUN) {
            const std::string fid = readVal();
            Type ftyp = static_cast<Type>(next());

            out << "FUN " << fid << " : " << Type_name(ftyp) << '\n';

            continue;
        }

        if (operation == OP::END) {

            out << "END\n\n";
            continue;
        }

        if (operation == OP::PARAM) {
            out << "    " << "PARAM ";
            OP kind = static_cast<OP>(next());
            out << (kind == OP::DEF ? "const " : "var ");
            const std::string vid = readVal();
            Type vtyp = static_cast<Type>(next());
            out << vid << " : " << Type_name(vtyp) << '\n';
            continue;
        }

        if (operation == OP::DEF) {
            const std::string vid = readVal();
            Type vtyp = static_cast<Type>(next());

            const std::string val = readVal();
            out << "    " << "DEF " << vid << " : " << Type_name(vtyp) << " = "
                << val << '\n';
            continue;
        }
        if (operation == OP::VAR) {
            const std::string vid = readVal();
            const auto vtyp = static_cast<Type>(next());
            out << "    " << "VAR " << vid << " : " << Type_name(vtyp) << '\n';
            continue;
        }

        if (operation == OP::STORE) {
            const std::string src = readVal();
            const std::string dst = readVal();
            out << "    " << "STORE " << src << " -> " << dst << '\n';
            continue;
        }

        if (OP::ADD <= operation &&
            operation <= OP::GE) { // binary arithmetic / comparison
            const std::string lhs = readVal();
            const std::string rhs = readVal();
            const std::string dst = readVal();
            out << "    " << op_name(operation) << ' ' << lhs << ", " << rhs
                << " -> " << dst << '\n';
            continue;
        }

        if (operation == OP::NEG) {
            const std::string operand = readVal();
            const std::string dst = readVal();
            out << "    " << "NEG " << operand << " -> " << dst << '\n';
            continue;
        }

        if (operation == OP::RET) {
            out << "    " << "RET " << readVal() << '\n';
            continue;
        }

        if (operation == OP::JMP) {
            const std::string target = readVal();
            out << "    " << "JMP " << target << '\n';
            continue;
        }
        if (operation == OP::JMPC) {
            const std::string cond = readVal();
            const std::string target = readVal();
            out << "    " << "JMPC " << cond << ", " << target << '\n';
            continue;
        }
        if (operation == OP::LABEL) {
            const std::string lbl = readVal();
            out << "label (" << lbl << "):\n";
            continue;
        }

        if (operation == OP::CALL) {
            const auto callType = static_cast<OP>(next());

            out << "    CALL ";

            switch (callType) {
            case OP::STD: {
                const auto lib = static_cast<std_lib>(next());
                const std::string arg = readVal();

                out << "STD " << std_name(lib) << ' ' << arg << '\n';
                break;
            }

            case OP::USR: {
                out << "USR " << readVal() << ' ';

                const auto arg_count = next();

                out << "argCount:" << arg_count << '(';

                for (int i = 0; i < arg_count; ++i)
                    out << readVal() << ' ';

                out << ") -> " << readVal() << '\n';
                break;
            }

            default:
                throw std::runtime_error("unknown call type!");
            }

            continue;
        }

        out << "[UNKNOWN 0x" << std::hex
            << static_cast<int>(static_cast<uint8_t>(operation)) << std::dec
            << " at offset " << (pos - 1) << "]\n";
        continue;
    }
}

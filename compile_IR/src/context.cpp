#include "context.hpp"

#include <stdexcept>

#include "navigateIR.hpp"
Id getId(const void *ptr) { return *(static_cast<const Id *>(ptr)); }
void debug_helper(const string &msg, const uint8_t *A, const uint8_t *B) {
    throw std::runtime_error(msg + " error at " +
                             std::to_string(static_cast<int>(B - A)) +
                             " byte\n");
}
Inst getInstuctionHead(OP op) {
    if ((OP::ADD <= op && op <= OP::MOD) || op == OP::NEG)
        return Inst::IMATH;
    if (OP::EQ <= op && op <= OP::GE)
        return Inst::ICMP;
    if (op == OP::JMP || op == OP::JMPC || op == OP::LABEL || op == OP::RET)
        return Inst::MOVE;
    if (op == OP::STORE)
        return Inst::STORE;
    if (op == OP::CALL)
        return Inst::UCALL;

    throw std::runtime_error("unsupported operator: " +
                             std::to_string(static_cast<int>(op)));
}
void function::parse() {
    if (static_cast<OP>(*it) != OP::FUN) {
        debug_helper("expected operator FUN at function start!", head, it);
    }
    skipOP(it);
    if (nextTag(it) != Tag::ID) {
        debug_helper("expected tag ID after operator FUN!", head, it);
    }
    name = nextId(it);
    type = nextType(it);

    while (peekOp(it) == OP::PARAM) {
        skipOP(it);
        if (nextOp(it) != OP::DEF) {
            debug_helper("expected expected operator DEF after op PARAM", head,
                         it);
        }
        if (nextTag(it) != Tag::ID) {
            debug_helper("only an Identifier can be a function parameter!",
                         head, it);
        }
        const variable v = {.name = nextId(it), .type = nextType(it)};
        params.push_back(v);

        local[v.name] = v;
    }

    while (peekOp(it) != OP::END) {
        parseInstuction();
    }
    skipOP(it);
}
void function::simpleInstuction(int arg_count) {
    const auto temp = static_cast<OP>(*it);
    skipOP(it);
    instruction i;

    i.head = getInstuctionHead(temp);
    i.option = static_cast<uint8_t>(temp);

    while (arg_count) {
        i.args.push_back(getValue());
        arg_count--;
    }

    i.result = getValue();

    instructions.push_back(i);
}
void function::functionCall() {
    // 1. Skip the OP::CALL byte itself
    skipOP(it);

    // 2. Read STD or USR
    const auto temp = nextOp(it);

    instruction i;
    if (temp == OP::STD) {
        i.head = Inst::SCALL;

        const auto lib = nextSTD(it);
        i.option = static_cast<uint8_t>(lib);

        if (lib != Lib::PRINT) {
            debug_helper("other libs are unsupported for now!", head, it);
        }

        // Parse the single printed argument (e.g. string or variable ID)
        i.args.push_back(getValue());
        instructions.push_back(i);
        return;

    } else if (temp == OP::USR) {
        i.head = Inst::UCALL;

        // Parse user function ID
        const auto value = getValue();
        if (value.type != Val_t::ID) {
            debug_helper("expected an UID after user call", head, it);
        }

        // Parse arg count
        const auto arg_count = *(it++);
        i.option = arg_count;

        // First argument is target function ID
        i.args.push_back(value);

        int t = 0;
        while (t < arg_count) {
            i.args.push_back(getValue()); // Read argument values
            t++;
        }

        // Parse target return variable (e.g. -> tag: v19)
        i.result = getValue();
        instructions.push_back(i);
        return;
    }

    debug_helper("expected the operator STD or USR after operator CALL", head,
                 it);
}
void function::declaration(bool isConst) {
    instruction i;
    i.head = Inst::DEC;

    i.option = *it;
    skipOP(it);
    const auto value = getValue();

    if (value.type != Val_t::ID) {
        debug_helper("expected an ID at var declaration", head, it);
    }
    const variable v = {
        .name = *(Id *)value.data, .type = nextType(it), .isConst = isConst};

    local[v.name] = v;

    i.result = value;
    if (isConst) {
        const auto val2 = getValue();
        i.args.push_back(val2);
    }
    instructions.push_back(i);
    return;
}
void function::parseInstuction() {

    if (!isOP(*it)) {
        debug_helper("expected a valid operator!", head, it);
    }

    const OP op = static_cast<OP>(*it);

    if (OP::ADD <= op &&
        op <= OP::GE) { // binary operators (2 args -> 1 result)
        simpleInstuction(2);
        return;
    }
    switch (op) {
    case OP::LABEL: {

        simpleInstuction(0);

        return;
    }
    case OP::RET: {

        simpleInstuction(0);

        return;
    }
    case OP::JMP: {
        simpleInstuction(0);
        return;
    }
    case OP::JMPC: {
        simpleInstuction(1);
        return;
    }
    case OP::STORE: {
        simpleInstuction(1);
        return;
    }
    case OP::VAR: {
        declaration(false);
        return;
    }
    case OP::DEF: {
        declaration(true);
        return;
    }
    case OP::CALL: {
        functionCall();
        return;
    }
    case OP::NEG: {
        simpleInstuction(1);
        return;
    }
    default:
        debug_helper("expected a valid operator!", head, it);
    }
}
struct match {
    bool found_match = false;
    Id A;
    Id B;
};
void function::resolve_types() {

    // build and array of types that should match
    std::vector<match> m_arr;
    for (const auto &i : instructions) {
        if (i.head == Inst::MOVE) {
            if (i.option == (uint8_t)OP::LABEL) {
                local.erase(getId(i.result.data));
            }
        }
        if (i.head == Inst::IMATH) {

            const auto C = i.result;

            const auto A = i.args[0];
            if (A.type == Val_t::ID) {
                match m = {.A = getId(C.data), .B = getId(A.data)};
                m_arr.push_back(m);
            }
            const auto B = i.args[1];
            if (B.type == Val_t::ID) {
                match m = {.A = getId(C.data), .B = getId(B.data)};
                m_arr.push_back(m);
            }
        }
        if (i.head == Inst::STORE) {
            const auto C = i.result;

            const auto A = i.args[0];
            if (A.type == Val_t::ID) {
                match m = {.A = getId(C.data), .B = getId(A.data)};
                m_arr.push_back(m);
            }
        }
        if (i.head == Inst::ICMP) {
            const auto res = getId(i.result.data);
            local[res].type = Type::i1;
        }
    }

    for (auto it = instructions.begin(); it != instructions.end();) {

        if (it->head == Inst::IMATH) {
            const auto C = it->result;

            const auto A = it->args[0];
            if (A.type == Val_t::ID &&
                local.find(*(static_cast<const Id *>(C.data))) != local.end()) {
                match m = {.A = *(static_cast<const Id *>(C.data)),
                           .B = *(static_cast<const Id *>(A.data))};
                m_arr.push_back(m);
            }
            const auto B = it->args[1];
            if (B.type == Val_t::ID &&
                local.find(*(static_cast<const Id *>(C.data))) != local.end()) {
                match m = {.A = *(static_cast<const Id *>(C.data)),
                           .B = *(static_cast<const Id *>(B.data))};
                m_arr.push_back(m);
            }
        }

        if (it->head == Inst::STORE) {
            const auto C = it->result;

            const auto A = it->args[0];
            if (A.type == Val_t::ID &&
                local.find(*(static_cast<const Id *>(C.data))) != local.end()) {
                match m = {.A = *(static_cast<const Id *>(C.data)),
                           .B = *(static_cast<const Id *>(A.data))};
                m_arr.push_back(m);
            }
        }

        if (it->head == Inst::ICMP) {
            const auto res = *static_cast<const Id *>(it->result.data);
            local[res].type = Type::i1;
        }

        ++it; // Move to the next element
    }

    bool repeat = true;
    while (repeat) {
        repeat = false;
        for (auto &m : m_arr) {
            if (m.found_match == false) {
                if (local[m.A].type != Type::unknown) {
                    m.found_match = true;
                    local[m.B].type = local[m.A].type;
                    continue;
                } else if (local[m.B].type != Type::unknown) {
                    m.found_match = true;
                    local[m.A].type = local[m.B].type;
                    continue;
                } else {
                    repeat = true;
                }
            }
        }
    }
}
Value function::getValue() {
    const auto tag = nextTag(it);
    const void *ptr = it;
    uint32_t size = 0;
    Val_t type;

    switch (tag) {
    case Tag::ID: {
        const Id var_name = *static_cast<const Id *>(ptr);
        const auto index = local.find(var_name);
        if (index == local.end()) {
            // add to locals with default values
            local[var_name] = variable{.name = var_name};
        }
        type = Val_t::ID;
        size = 4;
        it += 4;
        break;
    }

    case Tag::NUM8:
    case Tag::NUM16:
    case Tag::NUM32:
    case Tag::NUM64: {
        type = Val_t::NUM;
        size = (1 << static_cast<size_t>(static_cast<uint8_t>(tag) -
                                         static_cast<uint8_t>(Tag::NUM8)));
        it += size;
        break;
    }

    case Tag::STR8:
    case Tag::STR16:
    case Tag::STR32:
    case Tag::STR64: {
        type = Val_t::STR;
        const size_t byte_num =
            (1 << static_cast<size_t>(static_cast<uint8_t>(tag) -
                                      static_cast<uint8_t>(Tag::STR8)));

        // Read string length (little-endian)
        for (size_t i = 0; i < byte_num; ++i)
            size |= static_cast<uint64_t>(*it++) << (i << 3);

        // ptr points to the start of the actual string bytes
        ptr = it;
        it += size; // Advance iterator past string content
        break;
    }

    case Tag::VOID: {
        type = Val_t::NUL;
        size = 0;
        break;
    }

    default:
        debug_helper("unknown tag encountered in function getValue: " +
                         std::to_string(static_cast<int>(tag)),
                     head, it);
        size = 0;
        type = Val_t::NUL;
    }

    return Value{.type = type, .data = ptr, .size = size};
}

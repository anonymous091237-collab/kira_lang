#include "Type_resolver.hpp"

#include <stdexcept>

#include "../../include/debug.h"
#include "navigateIR.hpp"

void Resolver::build_type_context() {
    DEBUG_PRINT("\n\n\n\n\n");

    const uint8_t* tk = tokens->data();
    const uint8_t* end = tk + tokens->size();
    const uint8_t* start;
    string debug;
    while (tk < end) {
        OP op = nextOp(tk);
        start = tk;
        if (op == OP::FUN) {
            resolve_fun(tk);
            debug = "FUN";

        } else if (op == OP::VAR) {
            VarInfo v;

            Tag tag = nextTag(tk);

            if (tag != Tag::ID) {
                throw std::runtime_error("Expected VAR tag after VAR op");
            }

            v.id = nextId(tk);
            v.type = nextType(tk);
            v.is_pointer = true;

            (*variables)[v.id] = v;
            debug = "VAR";
        } else if (op == OP::DEF) {
            VarInfo v;

            Tag varTag = nextTag(tk);  // should be VAR

            if (varTag != Tag::ID) {
                throw std::runtime_error("Expected VAR tag after DEF op");
            }

            v.id = nextId(tk);
            v.type = nextType(tk);

            (*variables)[v.id] = v;

            Tag valueTag = nextTag(tk);

            if (valueTag == Tag::ID) {
                VarId id = nextId(tk);

                if (variables->contains(id)) {
                    Match m;
                    m.v1 = v.id;
                    m.v2 = id;
                    constraints.push_back(m);
                }
            } else if (Tag::STR8 <= valueTag && valueTag <= Tag::STR64) {
                saveString(nextStr(tk, valueTag));
            } else {
                skip(tk, valueTag);
            }
            debug = "DEF";
        } else if (op >= OP::ADD && op <= OP::MOD) {  // math operations
            resolve_math(tk);
            debug = "MATH";
        } else if (op >= OP::EQ && op <= OP::GE) {  // comparativeded
            resolve_cmp(tk);
            debug = "CMP";
        } else if (op == OP::PARAM) {
            throw std::runtime_error("PARAMS unsupported for now!\n");

        } else if (op == OP::NEG || op == OP::STORE) {  // comparativeded
                                                        /// EQ x 3 temp6
            VarId a;
            Tag tag = nextTag(tk);
            bool a_isvar = tag == Tag::ID;
            if (a_isvar) {
                a = nextId(tk);
            } else {
                skipNum(tk, tag);
            }

            VarInfo res;
            Tag tag2 = nextTag(tk);

            if (tag2 != Tag::ID) {
                throw std::runtime_error("Expected VAR tag after STORE/NEG op");
            }
            res.id = nextId(tk);

            if (!variables->contains(res.id)) {
                (*variables)[res.id] = res;
            }

            if (a_isvar && variables->contains(a)) {
                Match m;
                m.v1 = a;
                m.v2 = res.id;

                constraints.push_back(m);
            }
            debug = "NEG/STORE";
            //        END     RET,
        } else if (op == OP::JMPC) {  // JMPC tag v18, tag v15
            skip(tk, nextTag(tk));
            skip(tk, nextTag(tk));
            debug = "JMPC";
        } else if (op == OP::JMP) {  // JMP tag v14
            skip(tk, nextTag(tk));
            debug = "JMP";
        } else if (op == OP::LABEL) {  // label tag v16:
            skip(tk, nextTag(tk));
            debug = "LABEL";
        } else if (op == OP::CALL) {
            op = nextOp(tk);

            if (op == OP::STD) {
                std_lib std_fun = nextSTD(tk);

                if (std_fun == std_lib::PRINT) {
                    const Tag tag = nextTag(tk);

                    if (Tag::STR8 <= tag && tag <= Tag::STR64) {
                        saveString(nextStr(tk, tag));
                    } else {
                        skip(tk, tag);  // skip print arg}
                    }
                    debug = "CALL";
                }
            }

        } else if (op == OP::RET) {
            const Tag tag = nextTag(tk);
            skip(tk, tag);
            debug = "RET";

        } else if (op == OP::END) {
            debug = "END";
            DEBUG_PRINT("%s crossed %d bytes\n", debug.c_str(),
                        (int)(tk - start));
            return;
        }
        DEBUG_PRINT("%s crossed %d bytes\n", debug.c_str(), (int)(tk - start));
    }
}
/*


*/
void Resolver::saveString(const string& src) {
    if (!store_strings->contains(src)) {  // we found a new unique string

        string str;
        int size;
        generate_llvm_str(src, str, size);

        (*store_strings)[src] = string_offset;  // store strings offset

        flatStrArray->append(str);
        string_offset += size;
    }
}
/*


*/
void Resolver::resolve_fun(const uint8_t*& token) {
    // const uint8_t* start = token;
    FunInfo f;
    Tag tag = nextTag(token);
    if (tag != Tag::ID) {
        DEBUG_PRINT("tag :%d\n", (int)tag);
        throw std::runtime_error("Expected ID tag after FUN op");
    }
    f.id = nextId(token);
    f.type = nextType(token);

    DEBUG_PRINT("token :%d\n", static_cast<int>(*token));

    // DEBUG_PRINT("token :%s\n", tk->data());
    while (*token == static_cast<uint8_t>(OP::PARAM)) {
        if (nextOp(token) != OP::DEF) {
            throw std::runtime_error(
                "expected keyword DEF at function parameter!\n");
        }

        token += 7;  // skip id (5 bytes) + Type (1 byte)+ next op (1 byte)
        f.param_count++;
        // DEBUG_PRINT("last while intsruction token :%s\n",
        // op_name.at(static_cast<OP>(*token)).data());
    }

    // DEBUG_PRINT("token :%s\n",
    // op_name.at(static_cast<OP>(*token)).data());

    (*functions)[f.id] = f;
}
void Resolver::resolve_cmp(const uint8_t*& token) {
    /// EQ x 3 temp6
    VarId a, b;
    Tag tag1 = nextTag(token);
    bool a_isvar = tag1 == Tag::ID;

    if (a_isvar) {
        a = nextId(token);
    } else {
        skipNum(token, tag1);
    }
    Tag tag2 = nextTag(token);
    bool b_isvar = tag2 == Tag::ID;

    if (b_isvar) {
        b = nextId(token);
    } else {
        skipNum(token, tag2);
    }

    VarInfo res;
    Tag tag_dst = nextTag(token);
    if (tag_dst != Tag::ID) {
        throw std::runtime_error(
            "Expected VAR tag for the dst in a cmp command!\n");
    }
    res.id = nextId(token);
    res.type = Type::i1;

    if (!variables->contains(res.id)) {
        (*variables)[res.id] = res;
    }

    if (a_isvar && b_isvar && variables->contains(a) &&
        variables->contains(b)) {
        Match m;
        m.v1 = a;
        m.v2 = b;

        constraints.push_back(m);
    }
}
void Resolver::resolve_math(const uint8_t*& token) {
    // ADD tag v26, tag 81 -> tag v30
    VarId a, b;
    Tag tag1 = nextTag(token);
    bool a_isvar = tag1 == Tag::ID;
    if (a_isvar) {
        a = nextId(token);
    } else {
        skipNum(token, tag1);
    }
    Tag tag2 = nextTag(token);
    bool b_isvar = tag2 == Tag::ID;
    if (b_isvar) {
        b = nextId(token);
    } else {
        skipNum(token, tag2);
    }

    token++;  // skip this tag
    VarInfo res;
    res.id = nextId(token);

    if (!variables->contains(res.id)) {
        (*variables)[res.id] = res;
    }

    if (a_isvar && variables->contains(a)) {
        Match m;
        m.v1 = a;
        m.v2 = res.id;
        constraints.push_back(m);
    }
    if (b_isvar && variables->contains(b)) {
        Match m;
        m.v1 = b;
        m.v2 = res.id;

        constraints.push_back(m);
    }
}

void smart_resolve_Types(varTable& variables, vector<Match>& constraints) {
    bool repeat = true;
    while (repeat) {
        repeat = false;
        for (auto& cs : constraints) {
            VarInfo* v1 = &variables.at(cs.v1);
            VarInfo* v2 = &variables.at(cs.v2);

            if (v1->type == v2->type) {
                continue;
            } else if (v1->type != Type::unknown) {
                v2->type = v1->type;
                repeat = true;
            } else if (v2->type != Type::unknown) {
                v1->type = v2->type;
                repeat = true;
            }
        }
    }
}

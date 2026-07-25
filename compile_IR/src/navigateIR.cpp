#include "navigateIR.hpp"

#include <stdexcept>

//===========================================================
uint64_t nextNum(const uint8_t*& tk, Tag tag) {
    uint64_t value = 0;
    size_t size;

    switch (tag) {
        case Tag::NUM_I8:
            size = 1;
            break;
        case Tag::NUM_I16:
            size = 2;
            break;
        case Tag::NUM_I32:
            size = 4;
            break;
        case Tag::NUM_I64:
            size = 8;
            break;
        default:
            throw std::runtime_error("Invalid numeric Tag: " +
                                     std::to_string(static_cast<int>(tag)));
    }
    for (size_t i = 0; i < size; ++i, ++tk)
        value |= static_cast<uint64_t>(*tk) << (i << 3);
    return value;
}
//===========================================================
void skipNum(const uint8_t*& tk, Tag tag) {
    switch (tag) {
        case Tag::NUM_I8:
            tk += 1;
            break;
        case Tag::NUM_I16:
            tk += 2;
            break;
        case Tag::NUM_I32:
            tk += 4;
            break;
        case Tag::NUM_I64:
            tk += 8;
            break;
        default:
            throw std::runtime_error("skip Invalid Tag: " +
                                     std::to_string(static_cast<int>(tag)));
    }
}
//===========================================================
void skip(const uint8_t*& tk, Tag tag) {
    switch (tag) {
        case Tag::ID:
            tk += 4;
            break;

        case Tag::NUM_I8:
        case Tag::NUM_I16:
        case Tag::NUM_I32:
        case Tag::NUM_I64: {
            const size_t byte_num =
                (1 << static_cast<size_t>((uint8_t)tag - (uint8_t)Tag::NUM_I8));
            tk += byte_num;
            break;
        }
        case Tag::STR8:
        case Tag::STR16:
        case Tag::STR32:
        case Tag::STR64: {
            const size_t byte_num =
                (1 << static_cast<size_t>((uint8_t)tag - (uint8_t)Tag::STR8));

            uint64_t size = 0;
            for (size_t i = 0; i < byte_num; ++i)
                size |= static_cast<uint64_t>(*tk++)
                        << (i << 3);  // little-endian
            tk += size;
            break;
        }

        case Tag::VOID:
            tk += 0;
            break;
        default:
            throw std::runtime_error("Invalid Tag: " +
                                     std::to_string(static_cast<int>(tag)));
    }
}
//===========================================================
void skipStr(const uint8_t*& tk, Tag tag) {
    switch (tag) {
        case Tag::STR8:
        case Tag::STR16:
        case Tag::STR32:
        case Tag::STR64: {
            const size_t byte_num =
                1 << static_cast<size_t>((uint8_t)tag - (uint8_t)Tag::STR8);
            uint64_t str_size = 0;
            for (size_t i = 0; i < byte_num; ++i)
                str_size |= static_cast<uint64_t>(*tk++) << (i << 3);
            tk += str_size;
            break;
        }

        default:
            throw std::runtime_error("expected an STR tag");
    }
}
//===========================================================
string nextStr(const uint8_t*& tk, Tag tag) {
    switch (tag) {
        case Tag::STR8:
        case Tag::STR16:
        case Tag::STR32:
        case Tag::STR64: {
            const size_t byte_num =
                1 << static_cast<size_t>((uint8_t)tag - (uint8_t)Tag::STR8);

            uint64_t str_size = 0;
            for (size_t i = 0; i < byte_num; ++i)
                str_size |= static_cast<uint64_t>(*tk++) << (i << 3);
            tk += str_size;

            return string(reinterpret_cast<const char*>(tk - str_size),
                          static_cast<std::string::size_type>(str_size));
        }

        default:
            throw std::runtime_error("expected an STR tag");
    }
}
//===========================================================
VarId nextId(const uint8_t*& tk) {
    VarId id = static_cast<VarId>(tk[0]) | (static_cast<VarId>(tk[1]) << 8) |
               (static_cast<VarId>(tk[2]) << 16) |
               (static_cast<VarId>(tk[3]) << 24);
    tk += 4;
    return id;
}
//===========================================================
Type nextType(const uint8_t*& tk) { return static_cast<Type>(*tk++); }
Tag nextTag(const uint8_t*& tk) { return static_cast<Tag>(*tk++); }
OP nextOp(const uint8_t*& tk) { return static_cast<OP>(*tk++); }
std_lib nextSTD(const uint8_t*& tk) { return static_cast<std_lib>(*tk++); }
void skipTag(const uint8_t*& tk) { tk++; }
void skipType(const uint8_t*& tk) { tk++; }
void skipId(const uint8_t*& tk) { tk += 4; }
bool isOP(const OP src) { return (src <= OP::END); }
bool isOP(const uint8_t src) { return (static_cast<OP>(src) <= OP::END); }
//===========================================================
#pragma once

#include <cstdint>

using Id = uint32_t;
enum class Type : uint8_t {
    unknown,
    i1,
    i8,
    i16,
    i32,
    i64,
    u8,
    u16,
    u32,
    u64,
    ch,
    str
};
enum class OP : uint8_t {
    FUN,
    RET,
    VAR,
    STORE,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD, // math operation
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE, // comparison
    DEF,
    JMPC,
    JMP,
    LABEL,
    NEG,
    PARAM,
    CALL,
    STD,
    END,
    USR
};

enum class Tag : uint8_t {
    VOID,
    ID,
    NUM8,
    NUM16,
    NUM32,
    NUM64,

    STR8,
    STR16,
    STR32,
    STR64
};
enum class Lib : uint8_t { PRINT, READ, ALLOC, FREE };
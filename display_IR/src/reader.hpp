#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

enum class Tag : uint8_t;

class Reader {
   private:
    size_t pos = 0;

    bool done();
    uint8_t peek();
    uint8_t next();

    uint32_t readId();
    uint64_t readBytes(uint8_t bytes);
    std::string readVal(Tag* tag_out = nullptr);

   public:
    void disassemble(std::ostream& out);
    explicit Reader(const std::vector<uint8_t>& buffer) : buf(buffer) {}
    const std::vector<uint8_t>& buf;
};

enum class Tag : uint8_t {
    VOID,
    ID,
    NUM_I8,
    NUM_I16,
    NUM_I32,
    NUM_I64,

    STR8,
    STR16,
    STR32,
    STR64
};
enum class std_lib : uint8_t { PRINT };
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
    MOD,  // math operation
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,  // comparison
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
const std::array<const char*, 25> StrOp = {
    "FUN",   "RET", "VAR",   "STORE", "ADD", "SUB", "MUL", "DIV",  "MOD",
    "EQ",    "NE",  "LT",    "LE",    "GT",  "GE",  "DEF", "JMPC", "JMP",
    "LABEL", "NEG", "PARAM", "CALL",  "STD", "END", "USR"};
const std::array<const char*, 25> StrType = {"unknown", "i1",  "i8", "i16",
                                             "i32",     "i64", "u8", "u16",
                                             "u32",     "u64", "ch", "str"};

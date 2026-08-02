#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <unordered_map>

enum class NodeType : uint8_t {
  NONE,
  BIOP, // + - * / == >= ...
  UNOP, // -x !x
  NUM,  // 123 5.4 0
  STR,  //  'c', "hello"
  ID,   // identifiers
  CALL, // foo(a, b)

  VARDEC, // int x = 5
  FUNDEC, // int foo(...)
  PARAMS, // parameter list
  PARAM,  // one parameter
  TYPE,

  BLOCK,    // {...}
  RETSTMNT, // return x
  IFSTMNT,  // if (...)
  ELSE,
  WHILESTMNT, // while (...)
  FORSTMNT,   // for (...)

  ASSIGN, // x = 5 (if you don't treat it as BiOp)
  ARG,    // function call arguments
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

const std::unordered_map<std::string_view, Type> typeHashMap = {
    {"bool", Type::i1},    {"byte", Type::i8},
    {"ubyte", Type::u8},   {"short", Type::i16},
    {"ushort", Type::u16}, {"int", Type::i32},
    {"uint", Type::u32},   {"long", Type::i64},
    {"ulong", Type::u64}

};

const std::unordered_map<std::string_view, OP> biopHashMap = {
    {"+", OP::ADD}, {"-", OP::SUB}, {"*", OP::MUL}, {"/", OP::DIV},
    {"%", OP::MOD}, {"==", OP::EQ}, {"!=", OP::NE}, {"<", OP::LT},
    {"<=", OP::LE}, {">", OP::GT},  {">=", OP::GE},
};

const std::unordered_map<std::string_view, std_lib> std_functions = {
    {"print", std_lib::PRINT}};
const std::array<std::string_view, 20> NodeTypeNames = {
    "NONE", "BIOP",       "UNOP",     "NUM",    "STR",
    "ID",   "CALL",       "VARDEC",   "FUNDEC", "PARAMS",
    "ELSE", "WHILESTMNT", "FORSTMNT", "ASSIGN", "ARG"};
inline constexpr std::array<std::string_view, 12> varTypes = {
    "void", "byte", "ubyte", "short", "ushort", "int",
    "uint", "long", "ulong", "bool",  "char",   "string"};
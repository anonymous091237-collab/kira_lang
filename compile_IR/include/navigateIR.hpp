#pragma once
#include "common.hpp"
#include <cstdint>
#include <string>

using std::string;

//===========================================================
void skip(const uint8_t *&tk, Tag tag);
void skip(const uint8_t *&tk); // get the tag yourself
void skipOP(const uint8_t *&tk);
void skipTag(const uint8_t *&tk);
void skipType(const uint8_t *&tk);
//===========================================================
OP peekOp(const uint8_t *tk);
//===========================================================
uint64_t nextNum(const uint8_t *&tk, const Tag tag);
string nextStr(const uint8_t *&tk, Tag tag);
Id nextId(const uint8_t *&tk);
Type nextType(const uint8_t *&tk);
Tag nextTag(const uint8_t *&tk);
OP nextOp(const uint8_t *&tk);
Lib nextSTD(const uint8_t *&tk);
//===========================================================
bool isOP(const uint8_t src);
bool isOP(const OP src);
bool is_type_int(const Type &v);
bool isId(Tag t);
bool isNumber(Tag t);
bool isString(Tag t);
//===========================================================
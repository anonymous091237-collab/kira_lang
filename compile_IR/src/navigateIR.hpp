#pragma once
#include <cstdint>
#include <string>

#include "../../include/constants.hpp"
using std::string;

//===========================================================
void skip(const uint8_t*& tk, Tag tag);
void skipTag(const uint8_t*& tk);
void skipId(const uint8_t*& tk);
void skipType(const uint8_t*& tk);
void skipNum(const uint8_t*& tk, const Tag tag);
void skipStr(const uint8_t*& tk);
//===========================================================
uint64_t nextNum(const uint8_t*& tk, const Tag tag);
string nextStr(const uint8_t*& tk, Tag tag);
VarId nextId(const uint8_t*& tk);
Type nextType(const uint8_t*& tk);
Tag nextTag(const uint8_t*& tk);
OP nextOp(const uint8_t*& tk);
std_lib nextSTD(const uint8_t*& tk);
//===========================================================
bool isOP(const uint8_t src);
bool isOP(const OP src);
bool is_type_int(const Type& v);
//===========================================================
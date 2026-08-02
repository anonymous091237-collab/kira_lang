#pragma once

#include "common.hpp"
#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

using Id = uint32_t;

void append_op(std::vector<uint8_t> &buffer, OP value);
void append_tag(std::vector<uint8_t> &buffer, Tag value);
void append_std(std::vector<uint8_t> &buffer, std_lib value);
void append_type(std::vector<uint8_t> &buffer, Type value);

void append_id(std::vector<uint8_t> &buffer, Id var);
void append_str(std::vector<uint8_t> &buffer, const std::string_view &str);
void append_num(std::vector<uint8_t> &buffer, const std::string_view &num);

class Reusable {
  // size_t capacity = 16;
  size_t size = 0;
  std::string_view str;
  // max size 8bytes
  std::array<uint8_t, 8> data;
  Tag tag;

public:
  Reusable() = default;
  ~Reusable() = default;

  void set_str(const std::string_view &src_str);
  void set_var(Id var);

  void set_num(const std::string_view &num);
  void write_to(std::vector<uint8_t> &buffer);
};

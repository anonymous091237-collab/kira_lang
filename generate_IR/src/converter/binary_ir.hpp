

#include <cstdint>
#include <string>
#include <vector>

#include "../../../include/constants.hpp"

Tag int_helper(const std::string& num, uint64_t& dst);
void append_op(std::vector<uint8_t>& buffer, OP value);
void append_tag(std::vector<uint8_t>& buffer, Tag op);
void append_std(std::vector<uint8_t>& buffer, std_lib value);
void append_type(std::vector<uint8_t>& buffer, Type value);
void append_var(std::vector<uint8_t>& buffer, VarId var);
void append_str(std::vector<uint8_t>& buffer, const std::string_view& str);
void append_num(std::vector<uint8_t>& buffer, const std::string_view& num);

class Reusable {
    // size_t capacity = 16;
    size_t size = 0;
    std::string_view str;
    // max size 8bytes
    std::array<uint8_t, 8> ptr;
    Tag tag;

   public:
    Reusable() = default;
    ~Reusable() = default;

    void set_str(const std::string_view& src_str);
    void set_var(VarId var);

    void set_num(const std::string_view& num);
    void write_to(std::vector<uint8_t>& buffer);
};

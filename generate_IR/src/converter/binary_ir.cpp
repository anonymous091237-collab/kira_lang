#include "binary_ir.hpp"
#include <charconv>
#include <cstdint>
#include <stdexcept>

constexpr uint64_t MAX_8_BIT = 0xff;
constexpr uint64_t MAX_16_BIT = 0xffff;
constexpr uint64_t MAX_32_BIT = 0xffffffff;

Tag int_helper(const std::string_view &num, uint64_t &dst) {
    const auto [ptr, ec] =
        std::from_chars(num.data(), num.data() + num.size(), dst);

    if (ec != std::errc{} || ptr != num.data() + num.size()) {
        throw std::runtime_error("invalid integer");
    }

    if (dst <= MAX_8_BIT) {
        return Tag::NUM8;
    }
    if (dst <= MAX_16_BIT) {
        return Tag::NUM16;
    }
    if (dst <= MAX_32_BIT) {
        return Tag::NUM32;
    }
    return Tag::NUM64;
}

Tag str_helper(const uint64_t &src) {
    if (src <= MAX_8_BIT) {
        return Tag::STR8;
    }
    if (src <= MAX_16_BIT) {
        return Tag::STR16;
    }
    if (src <= MAX_32_BIT) {
        return Tag::STR32;
    }
    return Tag::STR64;
}

void append_op(std::vector<uint8_t> &buffer, OP value) {
    buffer.push_back(static_cast<uint8_t>(value));
}

void append_tag(std::vector<uint8_t> &buffer, Tag value) {
    buffer.push_back(static_cast<uint8_t>(value));
}

void append_std(std::vector<uint8_t> &buffer, std_lib value) {
    buffer.push_back(static_cast<uint8_t>(value));
}

void append_type(std::vector<uint8_t> &buffer, Type value) {
    buffer.push_back(static_cast<uint8_t>(value));
}

void append_id(std::vector<uint8_t> &buffer, Id var) {
    const size_t pos = buffer.size();
    buffer.resize(pos + 5);

    buffer[pos] = static_cast<uint8_t>(Tag::ID);
    buffer[pos + 1] = static_cast<uint8_t>(var);
    buffer[pos + 2] = static_cast<uint8_t>(var >> 8);
    buffer[pos + 3] = static_cast<uint8_t>(var >> 16);
    buffer[pos + 4] = static_cast<uint8_t>(var >> 24);
}

void append_str(std::vector<uint8_t> &buffer, const std::string_view &str) {
    uint64_t str_size = str.size();

    Tag stringtag = str_helper(str_size);
    buffer.push_back(static_cast<uint8_t>(stringtag));
    uint8_t bytenumber;
    if (Tag::STR8 <= stringtag && stringtag <= Tag::STR64) {
        bytenumber = 1 << (static_cast<uint8_t>(stringtag) -
                           static_cast<uint8_t>(Tag::STR8));
    } else {
        throw std::runtime_error("expected tag of type STR");
    }

    size_t start_idx = buffer.size(); // point to last element
    buffer.resize(buffer.size() + str_size + bytenumber);

    memcpy(buffer.data() + start_idx, &str_size,
           bytenumber); // copy string size

    start_idx += bytenumber;
    memcpy(buffer.data() + start_idx, str.data(),
           str_size); // copy the string data
}

void append_num(std::vector<uint8_t> &buffer, const std::string_view &num) {

    uint64_t number;
    Tag tag = int_helper(num, number);
    buffer.push_back(static_cast<uint8_t>(tag));

    uint8_t bytenumber;

    if (Tag::NUM8 <= tag && tag <= Tag::NUM64) {
        bytenumber =
            1 << (static_cast<uint8_t>(tag) - static_cast<uint8_t>(Tag::NUM8));
    } else {
        throw std::runtime_error("expected tag of type NUM");
    }

    size_t start_idx = buffer.size();
    buffer.resize(buffer.size() + bytenumber);
    memcpy(buffer.data() + start_idx, &number, bytenumber);
}

void Reusable::set_str(const std::string_view &src_str) {
    uint64_t str_size = src_str.size();
    tag = str_helper(str_size);

    uint8_t bytenumber;
    if (Tag::STR8 <= tag && tag <= Tag::STR64) {
        bytenumber =
            1 << (static_cast<uint8_t>(tag) - static_cast<uint8_t>(Tag::STR8));
    } else {
        throw std::runtime_error("expected tag of type STR");
    }

    memcpy(data.data(), &str_size, bytenumber);

    size = bytenumber;
    str = src_str;
}
void Reusable::set_var(Id var) {
    data[0] = static_cast<uint8_t>(var);
    data[1] = static_cast<uint8_t>(var >> 8);
    data[2] = static_cast<uint8_t>(var >> 16);
    data[3] = static_cast<uint8_t>(var >> 24);

    size = 4;
    tag = Tag::ID;
}

void Reusable::set_num(const std::string_view &num) {
    uint64_t number;
    tag = int_helper(num, number);

    uint8_t bytenumber;
    if (Tag::NUM8 <= tag && tag <= Tag::NUM64) {
        bytenumber =
            1 << (static_cast<uint8_t>(tag) - static_cast<uint8_t>(Tag::NUM8));
    } else {
        throw std::runtime_error("expected tag of type NUM");
    }

    memcpy(data.data(), &number, bytenumber);

    size = bytenumber;
}

void Reusable::write_to(std::vector<uint8_t> &buffer) {
    buffer.push_back(static_cast<uint8_t>(tag));
    size_t it = 0;
    while (it < size) {
        buffer.push_back(data[it]);
        it++;
    }
    if (Tag::STR8 <= tag && tag <= Tag::STR64) {
        size_t start_idx = buffer.size();
        buffer.resize(buffer.size() + str.size());
        memcpy(buffer.data() + start_idx, str.data(), str.size());
    }
}

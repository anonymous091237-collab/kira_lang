#include "binary_ir.hpp"

#include <iostream>
#include <stdexcept>

Tag int_helper(const std::string_view& num, uint64_t& dst) {
    dst = std::stoull(num.data());
    if (dst <= 0xff) {
        return Tag::NUM_I8;
    } else if (dst <= 0xffff) {
        return Tag::NUM_I16;
    } else if (dst <= 0xffffffff) {
        return Tag::NUM_I32;
    } else {
        return Tag::NUM_I64;
    }
}

Tag str_helper(const uint64_t& src) {
    if (src <= 0xff) {
        return Tag::STR8;
    } else if (src <= 0xffff) {
        return Tag::STR16;
    } else if (src <= 0xffffffff) {
        return Tag::STR32;
    } else {
        return Tag::STR64;
    }
}

void append_op(std::vector<uint8_t>& buffer, OP op) {
    buffer.push_back(static_cast<uint8_t>(op));
}
void append_tag(std::vector<uint8_t>& buffer, Tag op) {
    buffer.push_back(static_cast<uint8_t>(op));
}
void append_std(std::vector<uint8_t>& buffer, std_lib op) {
    buffer.push_back(static_cast<uint8_t>(op));
}
void append_type(std::vector<uint8_t>& buffer, Type op) {
    buffer.push_back(static_cast<uint8_t>(op));
}

void append_var(std::vector<uint8_t>& buffer, VarId var) {
    buffer.push_back(static_cast<uint8_t>(Tag::ID));
    buffer.push_back(static_cast<uint8_t>(var));
    buffer.push_back(static_cast<uint8_t>(var >> 8));
    buffer.push_back(static_cast<uint8_t>(var >> 16));
    buffer.push_back(static_cast<uint8_t>(var >> 24));
}

void append_str(std::vector<uint8_t>& buffer, const std::string_view& str) {
    uint64_t str_size = str.size();

    Tag stringtag = str_helper(str_size);
    buffer.push_back(static_cast<uint8_t>(stringtag));

    uint8_t bytenumber;
    switch (stringtag) {
        case Tag::STR8:
            bytenumber = 1;
            break;
        case Tag::STR16:
            bytenumber = 2;
            break;
        case Tag::STR32:
            bytenumber = 4;
            break;
        case Tag::STR64:
            bytenumber = 8;
            break;
        default:
            throw std::runtime_error("expected tag of type STR");
    }
    // make buffer big enough
    size_t start_idx = buffer.size();  // point to last element past buffer
    buffer.resize(buffer.size() + str_size + bytenumber);
    // copy string size
    memcpy(buffer.data() + start_idx, &str_size, bytenumber);
    // copy the string data
    start_idx += bytenumber;
    memcpy(buffer.data() + start_idx, str.data(), str_size);
    /* std::cout << "append str : " << str << '\n';

     std::cout << "append str size: " << str_size << '\n';
     std::cout << "append str byte count: " << (int)bytenumber << '\n';*/
}

void append_num(std::vector<uint8_t>& buffer, const std::string_view& num) {
    uint64_t x;
    Tag tag = int_helper(num, x);
    buffer.push_back(static_cast<uint8_t>(tag));

    uint8_t bytenumber;

    switch (tag) {
        case Tag::NUM_I8:
            bytenumber = 1;
            break;
        case Tag::NUM_I16:
            bytenumber = 2;
            break;
        case Tag::NUM_I32:
            bytenumber = 4;
            break;
        case Tag::NUM_I64:
            bytenumber = 8;
            break;
        default:
            throw std::runtime_error("expected tag of type NUM");
    }
    size_t start_idx = buffer.size();
    buffer.resize(buffer.size() + bytenumber);
    memcpy(buffer.data() + start_idx, &x, bytenumber);
}

void Reusable::set_str(const std::string_view& src_str) {
    uint64_t str_size = src_str.size();
    tag = str_helper(str_size);

    int bytenumber;

    switch (tag) {
        case Tag::STR8:
            bytenumber = 1;
            break;
        case Tag::STR16:
            bytenumber = 2;
            break;
        case Tag::STR32:
            bytenumber = 4;
            break;
        case Tag::STR64:
            bytenumber = 8;
            break;
        default:
            throw std::runtime_error("expected tag of type STR");
    }

    memcpy(ptr.data(), &str_size, bytenumber);

    size = bytenumber;
    str = src_str;
}
void Reusable::set_var(VarId var) {
    ptr[0] = static_cast<uint8_t>(var);
    ptr[1] = static_cast<uint8_t>(var >> 8);
    ptr[2] = static_cast<uint8_t>(var >> 16);
    ptr[3] = static_cast<uint8_t>(var >> 24);

    size = 4;
    tag = Tag::ID;
}

void Reusable::set_num(const std::string_view& num) {
    uint64_t x;
    tag = int_helper(num, x);
    int bytenumber;
    switch (tag) {
        case Tag::NUM_I8:
            bytenumber = 1;
            break;
        case Tag::NUM_I16:
            bytenumber = 2;
            break;
        case Tag::NUM_I32:
            bytenumber = 4;
            break;
        case Tag::NUM_I64:
            bytenumber = 8;
            break;
        default:
            throw std::runtime_error("expected tag of type NUM");
    }

    memcpy(ptr.data(), &x, bytenumber);

    size = bytenumber;
}
void Reusable::write_to(std::vector<uint8_t>& buffer) {
    buffer.push_back(static_cast<uint8_t>(tag));
    size_t it = 0;
    while (it < size) {
        buffer.push_back(ptr[it]);
        it++;
    }
    if (Tag::STR8 <= tag && tag <= Tag::STR64) {
        size_t start_idx = buffer.size();
        buffer.resize(buffer.size() + str.size());
        memcpy(buffer.data() + start_idx, str.data(), str.size());
    }
}
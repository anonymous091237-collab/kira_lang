#include "reader.hpp"

#include <cstdint>
#include <sstream>
#include <stdexcept>

constexpr uint8_t BITS_PER_BYTE = 8;
bool Reader::done() { return pos >= buf.size(); }

uint8_t Reader::peek() {
    if (done()) {
        throw std::runtime_error("unexpected end of file");
    }
    return buf[pos];
}

uint8_t Reader::next() {
    uint8_t tmp = peek();
    ++pos;
    return tmp;
}

uint32_t Reader::readId() {
    if (pos + 4 > buf.size()) {
        throw std::runtime_error("unexpected end of file");
    }
    uint32_t res = 0;
    for (uint8_t i = 0; i < 4; ++i) {
        res |= static_cast<uint32_t>(next())
               << (i * BITS_PER_BYTE); // little-endian
    }
    return res;
}

uint64_t Reader::readBytes(const uint8_t bytes) {
    if (pos + bytes > buf.size()) {
        throw std::runtime_error("unexpected end of file");
    }
    uint64_t res = 0;
    for (uint8_t i = 0; i < bytes; ++i) {
        res |= static_cast<uint64_t>(next())
               << (i * BITS_PER_BYTE); // little-endian
    }
    return res;
}

std::string Reader::readVal(Tag *tag_out) {
    const auto tag = static_cast<Tag>(next());
    if (tag_out != nullptr)
        *tag_out = tag;

    std::ostringstream os;
    os << "tag: ";
    switch (tag) {
    case Tag::VOID: {
        os << "<- void";
        break;
    }
    case Tag::ID: {
        os << "v" << readId();
        break;
    }
    case Tag::NUM_I8:
    case Tag::NUM_I16:
    case Tag::NUM_I32:
    case Tag::NUM_I64: {
        const uint8_t bytenum =
            (static_cast<uint8_t>(tag) - static_cast<uint8_t>(Tag::NUM_I8) + 1);
        os << readBytes(bytenum);
        break;
    }

    case Tag::STR8:
    case Tag::STR16:
    case Tag::STR32:
    case Tag::STR64: {
        const uint8_t bytenum =
            (static_cast<uint8_t>(tag) - static_cast<uint8_t>(Tag::STR8) + 1);

        const auto size = readBytes(bytenum);

        os << "strSize(" << size << ") \"";
        for (uint64_t i = 0; i < size; ++i) {
            os << (char)next();
        }
        os << '\"';
        break;
    }

    default:
        throw std::runtime_error("unknown Tag: " +
                                 std::to_string(static_cast<int>(tag)));
    }
    return os.str();
}

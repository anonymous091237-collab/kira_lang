#include "reader.hpp"

#include <sstream>
#include <stdexcept>

bool Reader::done() { return pos >= buf.size(); }

uint8_t Reader::peek() {
    if (done()) throw std::runtime_error("unexpected end of file");
    return buf[pos];
}

uint8_t Reader::next() {
    uint8_t b = peek();
    ++pos;
    return b;
}

uint32_t Reader::readId() {
    if (pos + 4 > buf.size())
        throw std::runtime_error("unexpected end of file");
    uint32_t t = 0;
    for (uint8_t i = 0; i < 4; ++i) {
        t |= (uint32_t)next() << (i * 8);  // little-endian
    }
    return t;
}
uint64_t Reader::readBytes(const uint8_t bytes) {
    if (pos + bytes > buf.size())
        throw std::runtime_error("unexpected end of file");
    uint64_t t = 0;
    for (uint8_t i = 0; i < bytes; ++i) {
        t |= (uint64_t)next() << (i * 8);  // little-endian
    }
    return t;
}

std::string Reader::readVal(Tag* tag_out) {
    const auto tag = static_cast<Tag>(next());
    if (tag_out != nullptr) *tag_out = tag;

    std::ostringstream os;
    os << "tag ";
    switch (tag) {
        case Tag::VOID: {
            os << "<- void";
            break;
        }
        case Tag::ID: {
            os << "v" << readId();
            break;
        }
        case Tag::NUM_I8: {
            os << readBytes(1);
            break;
        }
        case Tag::NUM_I16: {
            os << readBytes(2);
            break;
        }
        case Tag::NUM_I32: {
            os << readBytes(4);
            break;
        }
        case Tag::NUM_I64: {
            os << readBytes(8);
            break;
        }
        case Tag::STR8: {
            const auto size = readBytes(1);
            os << "size " << size << " \"";
            for (uint64_t i = 0; i < size; ++i) os << (char)next();
            os << '\"';
            break;
        }
        case Tag::STR16: {
            const auto size = readBytes(2);
            os << "size " << size << " \"";
            for (uint64_t i = 0; i < size; ++i) os << (char)next();
            os << '\"';
            break;
        }
        case Tag::STR32: {
            const auto size = readBytes(4);
            os << "size " << size << " \"";
            for (uint64_t i = 0; i < size; ++i) os << (char)next();
            os << '\"';
            break;
        }
        case Tag::STR64: {
            const auto size = readBytes(8);
            os << "size " << size << " \"";
            for (uint64_t i = 0; i < size; ++i) os << (char)next();
            os << '\"';
            break;
        }

        default:
            throw std::runtime_error("unknown Tag: " +
                                     std::to_string(static_cast<int>(tag)));
    }
    return os.str();
}
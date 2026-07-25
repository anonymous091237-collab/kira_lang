#pragma once
#include <vector>

#include "../../include/constants.hpp"

class Reader {
   private:
    size_t pos = 0;

    bool done();
    uint8_t peek();
    uint8_t next();

    uint32_t readId();
    uint64_t readBytes(const uint8_t bytes);
    std::string readVal(Tag* tag_out = nullptr);

   public:
    void disassemble(std::ostream& out);
    Reader(const std::vector<uint8_t>& buffer) : buf(buffer) {}
    const std::vector<uint8_t>& buf;
};

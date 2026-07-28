#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "reader.hpp"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cerr << "How to use:\n\treadIR <input.bin> [output.txt]\n\t"
                     "readIR <input.bin> (printed to console)\n";
        return 1;
    }
    const std::string path = argv[1];

    std::ifstream inputFile(path, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Error: cannot open '" << path << "'\n";
        return 1;
    }

    const std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(inputFile),
                                      {});

    inputFile.close();

    std::ofstream ouput_file;
    if (argc >= 3) {
        ouput_file.open(argv[2]);
        if (!ouput_file) {
            std::cerr << "Error: cannot open '" << argv[2] << "' for writing\n";
            return 1;
        }
    }
    std::ostream &out = (argc >= 3) ? ouput_file : std::cout;

    try {
        Reader reader(buffer);

        out << "; IR disassembly of: " << argv[1] << "\n\n";
        reader.disassemble(out);
    } catch (const std::exception &e) {
        std::cerr << "Parse error: " << e.what() << '\n';
        return 1;
    }

    if (argc >= 3) {
        std::cerr << "Written to " << argv[2] << '\n';
    }
    return 0;
}

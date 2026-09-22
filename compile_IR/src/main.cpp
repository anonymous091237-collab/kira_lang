#include "compile.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {

    if (argc < 2 || argc > 3) {
        std::cerr << "Usage:\n\tkllvm <IRfile> [output_name]\n";
        return 1;
    }

    const std::filesystem::path input_path(argv[1]);

    const std::string stem =
        (argc == 3) ? std::string(argv[2]) : input_path.stem().string();

    try {
        std::ofstream file("c:\\dev\\hello.ll");

        compile C(input_path.string(), file);
        C.G.run();
        std::cout << "========\nDEBUG\n========\n";
        C.G.print();
        std::cout << "========\nDEBUG\n========\n";
        C.fill_str_array();

        C.run();
        file.close();
    } catch (const std::exception &e) {
        std::cerr << "Compile error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
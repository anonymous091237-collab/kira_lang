
#include <filesystem>
#include <iostream>

#include "compiler.hpp"
#include "type_resolver.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "How to use it:\n\tkllvm.exe <IRfile> <output_name>";
        return 1;
    }

    const std::string file = argv[1];
    const std::string name = argv[2];

    //=====================================================================
    const std::filesystem::path inputPath(file);
    const auto dir = inputPath.parent_path();
    // const auto baseName = inputPath.stem().string();

    const auto output_path =
        (!dir.empty()) ? dir.string() + '\\' + name + ".ll" : name + ".ll";

    //=====================================================================
    try {
        LlvmBuilder g;
        // g.mainId = c.mainId;

        g.load_IR(file);

        Resolver r(&g.flat_str_array, &g.store_strings, &g.tokens, &g.variables,
                   &g.functions);

        r.build_type_context();

        smart_resolve_Types(g.variables, r.constraints);

#ifdef _DEBUG
        std::cout << "=======solved===================\n";
        for (const auto& [key, value] : g.variables) {
            std::cout << "var " << value.id << " : "
                      << Type_to_str.at(value.type) << '\n';
        }
#endif

        g.string_offset = r.string_offset;
        g.run();
        g.generate_ll(output_path);

    } catch (const std::exception& e) {
        std::cerr << "Compile error: " << e.what() << '\n';
    }

    //===========================================================
    const auto exe_path =
        (!dir.empty()) ? dir.string() + '\\' + name + ".exe" : name + ".exe";

    const auto compile_cmd = "clang -O3 " + output_path + " -o " + exe_path;

    std::system(compile_cmd.c_str());
    return 0;
}



#include <filesystem>
#include <iostream>

#include "../../include/debug.h"
#include "converter/convert.hpp"
#include "parser/parser.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        std::cerr << "How to use:\n\tmain.exe  <IRname>  <souceFile>  "
                     "-saveAST(optional)";
        return 1;
    }

    const std::string name = argv[1];
    const std::string file = argv[2];
    //=====================================================================
    bool saveAstFile = false;
    if (argc == 4) {
        if (argv[3] != std::string("-saveAST")) {
            std::cerr << "How to use:\n\tmain.exe  <IRname>  <souceFile>  "
                         "-saveAST(optional)";
            return 1;
        }
        saveAstFile = true;
    }
    //=====================================================================
    const std::filesystem::path inputPath(file);
    const auto dir = inputPath.parent_path();
    const auto baseName = inputPath.stem().string();

    //=====================================================================
    const auto build_path =
        (!dir.empty()) ? dir.string() + "\\k_build" : "k_build";

    const auto del_cmd =
        (!dir.empty())
            ? "if exist \"" + build_path + "\" rmdir /s /q \"" + build_path +
                  '\"'
            : "if exist " + build_path + " rmdir /s /q " + build_path;

    std::system(del_cmd.c_str());
    //=====================================================================
    const auto build_cmd = "mkdir " + build_path;
    DEBUG_PRINT("build path : %s\n" build_path);
    std::system(build_cmd.c_str());
    //=====================================================================
    try {
        Parser p(file);
        p.run();
        Converter c(p.tree->root, name);
        c.saveAST = saveAstFile;
        c.run();

    } catch (const std::exception& e) {
        std::cerr << "Compile error: " << e.what() << '\n';
    }

    return 0;
}
//"C:/Users/anony/projects/CLionProjects/kira_lang/generate_IR/build/main.exe"
//"C:\Users\anony\Documents\k_lang_project\k_build\result"
//"C:\Users\anony\Documents\k_lang_project\main.txt"

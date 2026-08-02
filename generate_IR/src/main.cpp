
#include <filesystem>
#include <iostream>

#include "convert.hpp"
#include "debug.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]) {

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
  // const auto baseName = inputPath.stem().string();

  //=====================================================================

  const std::filesystem::path build_path =
      (!dir.empty() ? dir / "k_build" : std::filesystem::path("k_build"));

  // Remove old directory if it exists
  if (std::filesystem::exists(build_path))
    std::filesystem::remove_all(build_path);

  // Create it — no shell, no syntax issues
  std::filesystem::create_directories(build_path);

  DEBUG_PRINT("build path : %s\n", build_path.string().c_str());

  //=====================================================================
  try {
    Parser prs(file);
    prs.run();
    Converter cnv(prs.tree->root, name);
    cnv.buildPath = build_path.string();
    cnv.saveAST = saveAstFile;
    cnv.run();

  } catch (const std::exception &e) {
    std::cerr << "Compile error: " << e.what() << '\n';
  }

  return 0;
}

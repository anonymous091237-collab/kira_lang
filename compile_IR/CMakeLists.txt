cmake_minimum_required(VERSION 3.20)

project(main)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Automatically find all .cpp and .hpp files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "include/*.hpp"
)

# 2. Create the executable using the discovered files
add_executable(kllvm ${SOURCES}
    include/common.hpp)

# 3. Tell the compiler where to look for include paths
target_include_directories(kllvm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 4. Apply compiler options
target_compile_options(kllvm PRIVATE -O3 -D_DEBUG -Wall -Wextra)

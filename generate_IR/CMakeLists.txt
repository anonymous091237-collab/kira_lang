cmake_minimum_required(VERSION 3.20)

project(main)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

add_executable(kirgen ${SOURCES}
    include/debug.hpp)
target_include_directories(kirgen PRIVATE include)

#-D_DEBUG
target_compile_options(kirgen PRIVATE -O3  -D_DEBUG )

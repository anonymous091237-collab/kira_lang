cmake_minimum_required(VERSION 3.20)

project(MyProject)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(kllvm ${SRC_FILES})

target_include_directories(kllvm PRIVATE src)
#-D_DEBUG
target_compile_options(kllvm PRIVATE  -Wall -Wextra -O3)

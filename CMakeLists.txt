cmake_minimum_required(VERSION 3.20)
project(kira_lang)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(generate_IR)
add_subdirectory(compile_IR)
add_subdirectory(view_IR)
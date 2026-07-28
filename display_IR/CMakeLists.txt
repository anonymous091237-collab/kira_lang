cmake_minimum_required(VERSION 3.20)

project(main)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")

add_executable(kireader ${SOURCES})

target_include_directories(kireader PRIVATE src)
#-D_DEBUG
target_compile_options(kireader PRIVATE -O3)

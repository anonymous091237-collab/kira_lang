cmake_minimum_required(VERSION 3.20)

project(main)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main
    main.cpp
    src/reader.cpp
    src/disassemble.cpp


)

target_include_directories(main PRIVATE src)
#-D_DEBUG
target_compile_options(main PRIVATE  -O3  )
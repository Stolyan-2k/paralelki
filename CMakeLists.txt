cmake_minimum_required(VERSION 3.10)
project(SinArraySum)

option(USE_FLOAT "Use float instead of double" OFF)

add_executable(sin_array_sum dz_1.cpp)

if(USE_FLOAT)
    target_compile_definitions(sin_array_sum PRIVATE USE_FLOAT)
endif()
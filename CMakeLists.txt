cmake_minimum_required(VERSION 3.25)
project(XexUnpack)

set(CMAKE_CXX_STANDARD 17)

include_directories(mspack)

add_library(XexUnpack SHARED XexUnpack.cpp mspack/lzxd.c mspack/system.c)

set_target_properties(
        XexUnpack
        PROPERTIES
        OUTPUT_NAME "XexUnpack"
        OUTPUT_NAME.PREFIX "lib"
)

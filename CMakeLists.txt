cmake_minimum_required(VERSION 3.25)
project(libXexUnpack)

set(CMAKE_CXX_STANDARD 17)

include_directories(mspack)

add_library(libXexUnpack SHARED libXexUnpack.cpp mspack/lzxd.c mspack/system.c)

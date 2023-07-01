cmake_minimum_required(VERSION 3.25)
project(XexUnpack)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
endif()

include_directories(mspack)

add_library(XexUnpack SHARED XexUnpack.cpp mspack/lzxd.c mspack/system.c)

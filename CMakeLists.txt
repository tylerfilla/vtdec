cmake_minimum_required(VERSION 3.5)
project(vtparse)

set(vtparse_INCLUDE_FILES
        vtparse.h
        )

set(vtparse_SRC_FILES
        vtparse.c
        )

add_library(vtparse STATIC ${vtparse_INCLUDE_FILES} ${vtparse_SRC_FILES})
target_include_directories(vtparse PRIVATE .)

cmake_minimum_required(VERSION 3.15)
project(lzz3 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG -O4)

add_executable(lzz3 src/core/lib.c src/core/lib.h src/cli/main.c src/core/lzz.c src/core/lzz.h src/core/array_deque.c src/core/array_deque.h src/core/lzz_d.c src/core/lzz_d.h src/cli/test.c src/core/huffman_c.c src/core/huffman_c.h src/core/huffman_d.c src/core/huffman_d.h)
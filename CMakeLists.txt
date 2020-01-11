cmake_minimum_required(VERSION 3.15)
project(lzz3 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG -O2)

add_executable(lzz3 lib.c lib.h main.c lzz.c lzz.h array_deque.c array_deque.h lzz_d.c lzz_d.h test.c)
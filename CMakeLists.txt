cmake_minimum_required(VERSION 3.0)

project(trees LANGUAGES C)

set (cflags -g)
set(CMAKE_C_FLAGS ${cflags})


add_executable(convert convert.c exoquant.c)
target_link_libraries(convert m)


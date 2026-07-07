cmake_minimum_required(VERSION 3.26)
project(dungeon_generator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/Zc:preprocessor)
endif()

add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)

add_subdirectory(3D-engine)
add_subdirectory(dungeon-generator)
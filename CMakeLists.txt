cmake_minimum_required(VERSION 3.5)
project(morpho_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(morpho_engine
    src/main.cpp
    src/morpho.cpp
)

target_include_directories(morpho_engine PRIVATE src)

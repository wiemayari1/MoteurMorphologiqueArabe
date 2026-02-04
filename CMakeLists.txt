
cmake_minimum_required(VERSION 3.10)
project(MoteurMorphologiqueArabe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Exécutable console : morpho_engine ---
add_executable(morpho_engine
    src/main.cpp
    src/morpho.cpp
    src/AVL.cpp
    src/hash_table.cpp
)

target_include_directories(morpho_engine PRIVATE include)

# --- Exécutable tests unitaires ---
add_executable(tests
    src/tests.cpp
    src/morpho.cpp
    src/AVL.cpp
    src/hash_table.cpp
)

target_include_directories(tests PRIVATE include)

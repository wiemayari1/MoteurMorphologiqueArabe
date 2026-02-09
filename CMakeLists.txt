cmake_minimum_required(VERSION 3.10)
project(MoteurMorphologiqueArabe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Exécutable API/CLI : morpho_engine ---
add_executable(morpho_engine
    src/cli_main.cpp
    src/morpho.cpp
    src/AVL.cpp
    src/hash_table.cpp
)
target_include_directories(morpho_engine PRIVATE include)

# --- Exécutable console interactif (ton menu actuel) ---
add_executable(morpho_tui
    src/main.cpp
    src/morpho.cpp
    src/AVL.cpp
    src/hash_table.cpp
)
target_include_directories(morpho_tui PRIVATE include)

# --- Tests unitaires ---
add_executable(tests
    src/tests.cpp
    src/morpho.cpp
    src/AVL.cpp
    src/hash_table.cpp
)
target_include_directories(tests PRIVATE include)

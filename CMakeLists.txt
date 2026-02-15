cmake_minimum_required(VERSION 3.14)
project(MoteurMorphologiqueArabe VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(Threads REQUIRED)

# ===== SOURCES COMMUNES =====
set(CORE_SOURCES
    src/AVL.cpp
    src/hash_table.cpp
    src/morpho.cpp
)

set(CORE_HEADERS
    include/AVL.h
    include/hash_table.h
    include/morpho.h
)

# ===== EXÉCUTABLE 1: CLI (existant) =====
add_executable(moteur_cli
    src/cli_main.cpp
    ${CORE_SOURCES}
)

target_include_directories(moteur_cli PRIVATE include)
target_link_libraries(moteur_cli PRIVATE Threads::Threads)

# ===== EXÉCUTABLE 2: API SERVER (nouveau) =====
add_executable(morpho_api_server
    src/server_main.cpp
    src/http_server.cpp
    src/api_routes.cpp
    ${CORE_SOURCES}
)

target_include_directories(morpho_api_server PRIVATE include)
target_link_libraries(morpho_api_server PRIVATE Threads::Threads pthread)

# ===== EXÉCUTABLE 3: TESTS (si tu en as) =====
# add_executable(tests src/tests.cpp ${CORE_SOURCES})
# target_include_directories(tests PRIVATE include)

# ===== OPTIONS DE COMPILATION =====
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(moteur_cli PRIVATE -Wall -Wextra -O2)
    target_compile_options(morpho_api_server PRIVATE -Wall -Wextra -O3)
elseif(MSVC)
    target_compile_options(moteur_cli PRIVATE /W4 /O2)
    target_compile_options(morpho_api_server PRIVATE /W4 /O2)
endif()

# ===== INSTALLATION =====
install(TARGETS morpho_api_server moteur_cli
    RUNTIME DESTINATION bin
)

# Message
message(STATUS "✅ Configuration terminée")
message(STATUS "   Targets: moteur_cli, morpho_api_server")

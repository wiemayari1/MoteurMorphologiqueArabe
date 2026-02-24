cmake_minimum_required(VERSION 3.14)
project(MoteurMorphologiqueArabe VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # utile pour les IDE / clangd

find_package(Threads REQUIRED)

# ===== Chemin absolu vers les données =====
# Permet aux exécutables de trouver data/ quel que soit le répertoire de lancement
add_compile_definitions(
    DATA_DIR="${CMAKE_SOURCE_DIR}/data"
)

# ===== SOURCES COMMUNES (structures de données + moteur morphologique) =====
set(CORE_SOURCES
    src/AVL.cpp
    src/hash_table.cpp
    src/morpho.cpp
)

# ===== EXÉCUTABLE 1 : CLI simple =====
add_executable(moteur_cli
    src/cli_main.cpp
    ${CORE_SOURCES}
)
target_include_directories(moteur_cli PRIVATE include)
target_link_libraries(moteur_cli PRIVATE Threads::Threads)

# ===== EXÉCUTABLE 2 : TUI interactif (menu coloré) =====
add_executable(moteur_tui
    src/main.cpp
    ${CORE_SOURCES}
)
target_include_directories(moteur_tui PRIVATE include)
target_link_libraries(moteur_tui PRIVATE Threads::Threads)

# ===== EXÉCUTABLE 3 : Serveur API HTTP =====
add_executable(morpho_api_server
    src/server_main.cpp
    src/http_server.cpp
    src/api_routes.cpp
    ${CORE_SOURCES}
)
target_include_directories(morpho_api_server PRIVATE include)
target_link_libraries(morpho_api_server PRIVATE Threads::Threads)

# Sur Windows, lier ws2_32 pour les sockets (Winsock2)
if(WIN32)
    target_link_libraries(morpho_api_server PRIVATE ws2_32)
endif()

# ===== EXÉCUTABLE 4 : Tests unitaires =====
add_executable(moteur_tests
    src/tests.cpp
    ${CORE_SOURCES}
)
target_include_directories(moteur_tests PRIVATE include)

# ===== OPTIONS DE COMPILATION =====
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(target moteur_cli moteur_tui morpho_api_server moteur_tests)
        target_compile_options(${target} PRIVATE -Wall -Wextra -O2)
    endforeach()
elseif(MSVC)
    foreach(target moteur_cli moteur_tui morpho_api_server moteur_tests)
        # /utf-8 : encodage source + exécution en UTF-8 (essentiel pour l'arabe)
        target_compile_options(${target} PRIVATE /W4 /O2 /utf-8)
    endforeach()
endif()

# ===== Copie automatique du dossier data/ dans build/ =====
# Permet d'exécuter les binaires directement depuis build/ sans chemin relatif
add_custom_command(
    TARGET moteur_cli POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/data"
            "$<TARGET_FILE_DIR:moteur_cli>/data"
    COMMENT "Copie de data/ vers le repertoire de sortie"
)

# ===== INSTALLATION =====
install(TARGETS morpho_api_server moteur_cli moteur_tui
    RUNTIME DESTINATION bin
)
install(DIRECTORY data/ DESTINATION share/moteur-morphologique/data)

message(STATUS "Configuration terminée pour : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compilateur : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Cibles      : moteur_cli, moteur_tui, morpho_api_server, moteur_tests")

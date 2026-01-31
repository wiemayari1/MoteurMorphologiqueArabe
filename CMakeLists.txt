cmake_minimum_required(VERSION 3.5)
project(morpho_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajout de l'exécutable avec les fichiers sources
add_executable(morpho_engine
    src/main.cpp
    src/morpho.cpp
)

# Ajouter le répertoire des en-têtes
target_include_directories(morpho_engine PRIVATE 
    include  # Chemin vers les fichiers d'en-tête
    src      # Si nécessaire, ajoutez aussi le répertoire src
)

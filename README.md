# Moteur Morphologique Arabe

Ce projet est un moteur de traitement morphologique pour la langue arabe permettant de générer et de valider des formes verbales et nominales à partir de racines trilitères et de schèmes. Il se compose d'un noyau performant écrit en C++ et d'une interface utilisateur moderne développée avec Next.js.

## Fonctionnement technique

Le moteur repose sur deux structures de données principales pour optimiser les performances :
- **Arbre AVL** : Utilisé pour le stockage et la recherche rapide des racines (complexité logarithmique). Le support de l'arabe est assuré par une manipulation en UTF-32.
- **Table de hachage** : Utilisée pour l'accès instantané aux schèmes morphologiques via une implémentation à adressage ouvert.

## Organisation du projet

- `src/` : Code source C++ (moteur morphologique, API HTTP, outils CLI).
- `include/` : Fichiers d'en-tête (.h).
- `data/` : Fichiers de données au format texte et JSON pour les racines et les schèmes.
- `frontend/` : Application Web (React / Next.js).
- `CMakeLists.txt` : Script de configuration pour la compilation C++.

## Compilation et Installation

### Backend (C++)

Le backend nécessite un compilateur compatible C++17 et l'outil CMake.

#### Sur Windows
Il est recommandé d'utiliser Visual Studio et PowerShell :
```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
.\Release\morpho_api_server.exe
```

#### Sur Ubuntu / Linux
```bash
sudo apt update && sudo apt install build-essential cmake
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
./morpho_api_server
```

#### Sur WSL
Suivre la procédure Linux classique après avoir installé les outils de compilation (`build-essential`). L'API sera accessible sur `localhost`.

### Frontend (Next.js)

Le frontend nécessite l'installation de Node.js.

```bash
cd frontend
npm install
npm run dev
```

Une fois lancé, le dashboard est accessible sur `http://localhost:3000`.

## Utilisation

Le serveur API doit être lancé pour que l'interface web puisse fonctionner. Une interface en ligne de commande (CLI) est également disponible pour les tests rapides directement dans le terminal.

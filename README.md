# Moteur Morphologique Arabe

La langue arabe repose sur un système morphologique appelé **racine–schème** (Root–Pattern) : les mots ne se forment pas par ajout de préfixes ou de suffixes, mais en insérant une racine consonantique dans un schème morphologique abstrait.

Par exemple, la racine **ك–ت–ب** (écrire) combinée au schème **مفعول** donne **مكتوب** (écrit), et avec **فاعل** donne **كاتب** (écrivain).

Ce projet implémente un moteur capable d'exploiter ce mécanisme de manière algorithmique :
- les racines trilitères sont indexées dans un **arbre AVL** pour une recherche en O(log n),
- les schèmes morphologiques sont stockés dans une **table de hachage** pour un accès direct,
- le moteur peut **générer** des mots dérivés à partir d'une racine et d'un schème, et **valider** si un mot appartient morphologiquement à une racine donnée.

Le tout est écrit en **C++17**, sans bibliothèque externe, avec une prise en charge native de l'encodage **UTF-8/UTF-32** pour les caractères arabes.

---

## Fonctionnalités implémentées

Conformément au cahier des charges, le projet couvre les cinq fonctionnalités principales :

| Fonctionnalité | Implémentation |
|---|---|
| Gestion des racines (insertion, recherche, affichage) | Arbre AVL — `src/AVL.cpp` |
| Gestion des schèmes morphologiques | Table de hachage — `src/hash_table.cpp` |
| Génération morphologique (racine + schème → mot) | `apply_template()` — `src/morpho.cpp` |
| Validation morphologique (mot → racine + schème) | `validate_word()` — `src/morpho.cpp` |
| Gestion des dérivés validés par racine | Nœuds AVL avec liste de dérivés et fréquences |

**Fonctionnalités additionnelles :**
- Mini-jeu morphologique interactif dans le CLI
- Serveur API REST HTTP (sans dépendance externe)
- Interface web (Next.js) avec pages de génération, validation et jeu

---

## Structures de données

- **Arbre AVL** : chaque nœud stocke une racine trilitère arabe, la liste de ses dérivés validés et leur fréquence d'apparition. Les comparaisons utilisent l'encodage UTF-32 pour un traitement correct des caractères arabes.
- **Table de hachage** (adressage ouvert) : clé = nom du schème (ex. `مفعول`), valeur = représentation abstraite du schème avec sa règle de transformation algorithmique.

---

## Exemple de génération

```
Racine  : ك–ت–ب
Schème  : مفعول  (template : م–ف–ع–و–ل)
Résultat: مكتوب
```

La convention de transformation est : `ف` → 1ʳᵉ lettre, `ع` → 2ᵉ lettre, `ل` → 3ᵉ lettre de la racine.

---

## Structure du projet

```
MoteurMorphologiqueArabe/
├── src/
│   ├── AVL.cpp            Arbre AVL (gestion des racines)
│   ├── hash_table.cpp     Table de hachage (schèmes)
│   ├── morpho.cpp         Moteur de génération et de validation
│   ├── cli_main.cpp       Interface CLI interactive
│   ├── main.cpp           Interface TUI (menus colorés)
│   ├── server_main.cpp    Point d'entrée du serveur API
│   ├── http_server.cpp    Serveur HTTP minimal (sockets POSIX/Winsock2)
│   ├── api_routes.cpp     Routes REST (génération, validation, jeu)
│   └── tests.cpp          Tests unitaires
├── include/               En-têtes C++
├── data/
│   ├── roots.txt          Racines trilitères arabes
│   ├── schemes.txt        Schèmes morphologiques
│   └── derivatives.json   Dérivés validés par racine
├── frontend/              Interface web Next.js
├── CMakeLists.txt         Configuration CMake
└── Makefile               Wrapper Make (Linux / WSL)
```

---

## Compilation

### Prérequis

- **Ubuntu natif** : `build-essential`, `cmake` >= 3.14
- **WSL (depuis PowerShell)** : mêmes outils, installés dans l'environnement Linux

### Option 1 — Ubuntu natif

```bash
sudo apt update && sudo apt install -y build-essential cmake
cd MoteurMorphologiqueArabe
make
```

### Option 2 — WSL depuis PowerShell

```powershell
wsl
```

```bash
sudo apt update && sudo apt install -y build-essential cmake
cd /mnt/c/Users/MSI/MoteurMorphologiqueArabe
make
```

---

## Utilisation

```bash
cd build
./moteur_cli ../data/roots.txt ../data/schemes.txt
```

---

## Commandes Make

| Commande | Description |
|---|---|
| `make` | Compile en mode Release |
| `make debug` | Compile en mode Debug |
| `make clean` | Supprime le répertoire `build/` |
| `make run-cli` | Lance le CLI |
| `make run-tui` | Lance le TUI interactif |
| `make run-api` | Lance le serveur API |
| `make run-tests` | Lance les tests unitaires |
| `make frontend-dev` | Lance le serveur Next.js |

---

## Licence

Distribué sous la licence Apache-2.0. Voir le fichier [LICENSE](LICENSE).

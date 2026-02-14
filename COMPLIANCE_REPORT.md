# Rapport de Conformité au Cahier des Charges
**Projet : Moteur Morphologique Arabe**

Ce document certifie la conformité de la solution technique actuelle aux exigences du mini-projet algorithmique.

## I. Structures de Données

### 1.1 Arbre de gestion des racines (ABR/AVL)
**Exigence :** Stockage des racines dans un arbre AVL. Chaque nœud contient la racine, la liste des dérivés validés et leur fréquence.
**Preuve dans le code (`src/AVL.h`, `src/AVL.cpp`) :**
-   **Classe `AVLTree`** : Implémente un arbre équilibré avec rotations (`rotateLeft`, `rotateRight`).
-   **Structure `AVLNode`** :
    ```cpp
    struct AVLNode {
        std::u32string key;          // La racine
        std::vector<std::u32string> derived; // Liste des dérivés validés
        int frequency;               // Fréquence
        // ...
    };
    ```
-   **Conformité :** 100%

### 1.2 Dictionnaire des schèmes (Table de Hachage)
**Exigence :** Implémentation manuelle, accès rapide, clé=nom, valeur=schème.
**Preuve dans le code (`src/hash_table.cpp`) :**
-   **Classe `HashTable`** : Implémentation manuelle (pas de `std::unordered_map`).
-   **Résolution des collisions** : Adressage ouvert (Linear Probing).
-   **Fonction de hachage** : Algorithme FNV-1a pour chaînes UTF-32.
-   **Conformité :** 100%

---

## II. Fonctionnalités Principales

### 2. Gestion des racines
**Exigences :** Chargement fichier, insertion dynamique, recherche efficace, affichage structuré.
**Implémentation :**
-   **Chargement** : `load_roots_into_avl` (dans `cli_main.cpp`) lit `roots.txt` au démarrage.
-   **Insertion dynamique** : Commande `--add-root` (CLI) et route `POST /api/roots` (Web) -> mise à jour fichier + redémarrage moteur.
-   **Affichage** : Commande `--list-roots`.
-   **Conformité :** 100%

### 3. Génération morphologique
**Exigence :** Racine + Schème -> Mot dérivé.
**Implémentation :**
-   **Fonction** : `apply_template` (dans `src/morpho.cpp`).
-   **Commande** : `--generate` (renvoie JSON `{ok:true, word:"..."}`).
-   **Conformité :** 100%

### 4. Validation morphologique
**Exigence :** Mot + Racine -> OUI/NON + Schème.
**Implémentation :**
-   **Fonction** : `extract_root_from_word` (mathématique inverse).
-   **Commande** : `--validate` (renvoie JSON `{ok:true, belongs:true, schemes:[...]}`).
-   **Conformité :** 100%

### 5. Gestion des dérivés validés (Mise à jour dynamique)
**Exigence :** Association racine <-> dérivés, mise à jour automatique lors de génération/validation.
**Implémentation (Ajout récent) :**
-   Dans `cli_main.cpp`, boucle serveur :
    -   **Sur succès Generate** : Appelle `tree.addDerived(...)` et `tree.incrementFrequency(...)`.
    -   **Sur succès Validate** : Appelle `tree.addDerived(...)` et `tree.incrementFrequency(...)`.
-   **Conformité :** 100% (Logique respectée en mémoire vive du moteur).

---

## III. Interface et Bonus

### Interface Web (Angular)
-   Respecte le design existant (aucune modification CSS/HTML).
-   Communique avec le moteur via une API Node.js optimisée (`spawn` persistant).
-   **Correction** : Format JSON aligné pour éviter les "résultats vides".

### Bonus "Jeu"
-   Génération de questions QCM (Racine/Schème/Mot intrus).
-   Functionalité validée via `--game` et interface web.

---

## Conclusion
Le projet est **fonctionnel**, **optimisé** (mode serveur C++) et **strictement conforme** aux structures de données et algorithmes exigés par le cahier des charges.

**Pour tester la version finale :**
1. `git pull origin wiem-dev`
2. `cd build && cmake .. && make`
3. `pm2 restart all`

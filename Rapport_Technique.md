# Rapport Technique - Moteur Morphologique Arabe

**Année universitaire :** 2025-2026  
**Enseignants responsables :** Narjes Ben Hariz, Sahbi Bahroun  
**Niveau :** 1ING  
**Module :** Algorithmique et Structures de Données

---

## 1. Introduction
Ce projet a pour objectif de développer un moteur morphologique pour la langue arabe, capable d'analyser et de générer des mots à partir de racines trilitères et de schèmes. Il repose sur des structures de données avancées pour garantir performance et robustesse.

## 2. Structures de Données Utilisées

### 2.1 Arbre AVL pour les Racines
Nous avons choisi un **Arbre AVL** (Arbre Binaire de Recherche Équilibré) pour stocker les racines trilitères.
*   **Pourquoi ?** L'AVL garantit une hauteur logarithmique ($O(\log n)$), ce qui assure une recherche, une insertion et une suppression très rapides, même avec un grand nombre de racines.
*   **Structure du Nœud (`include/AVL.h`)** :
    *   `key` (`std::u32string`) : La racine (ex: "كتب").
    *   `derived` (`std::vector`) : Liste des dérivés validés associés.
    *   `frequency` (`int`) : Compteur d'utilisation de la racine.
    *   `height` (`int`) : Pour le maintien de l'équilibre.
*   **Opérations** : Insertion avec rotations (gauche/droite) pour maintenir l'équilibre. Suppression implémentée avec rééquilibrage.

### 2.2 Table de Hachage pour les Schèmes
Les schèmes sont gérés via une **Table de Hachage** implémentée manuellement.
*   **Pourquoi ?** L'accès à un schème par son nom doit être instantané ($O(1)$ en moyenne).
*   **Gestion des Collisions** : Nous avons opté pour l'**Adressage Ouvert avec Sondage Linéaire** (Linear Probing), efficace pour cette taille de données et évitant l'allocation dynamique de listes chaînées.
*   **Redimensionnement** : La table double de taille automatiquement lorsque le facteur de charge dépasse 0.7, garantissant des performances constantes.

## 3. Algorithmes Principaux

### 3.1 Génération Morphologique
L'algorithme de génération (`apply_template` dans `src/morpho.cpp`) prend une racine (ex: "كتب") et un template (ex: "مَ1ْ2ُو3").
1.  Il parcourt le template caractère par caractère.
2.  Si le caractère est un chiffre ('1', '2', '3'), il est remplacé par la lettre correspondante de la racine.
3.  Sinon, le caractère du template est conservé (lettres augmentées).

### 3.2 Validation Morphologique
L'algorithme inverse (`extract_root_from_word`) tente de déduire la racine à partir d'un mot et d'un schème candidat.
1.  Il superpose le mot et le template.
2.  Si les caractères fixes du template correspondent au mot, il extrait les lettres aux positions '1', '2', '3'.
3.  Si les lettres extraites forment une racine valide (présente dans l'AVL), le mot est validé.

## 4. Analyse de la Complexité

*   **Recherche de racine** : $O(\log N)$ où $N$ est le nombre de racines (grâce à l'AVL).
*   **Accès à un schème** : $O(1)$ en moyenne (Table de Hachage).
*   **Génération d'un mot** : $O(L)$ où $L$ est la longueur du template (négligeable et constant).
*   **Validation d'un mot** : $O(S \times \log N)$, où $S$ est le nombre de schèmes (on teste tous les schèmes potentiels) et $\log N$ est la vérification de la racine.

## 5. Fonctionnalités et Interface

### 5.1 Interface Ligne de Commande (CLI) - 100% Conforme
Le programme principal (`src/main.cpp`) offre un menu interactif complet :
*   Gestion complète des Racines (Ajout, **Suppression**, Recherche).
*   Gestion complète des Schèmes (Ajout, Modification, Suppression).
*   Génération et Validation.
*   Mini-jeu interactif en mode console.

### 5.2 Extensions : Interface Graphique et Jeu (Bonus)
Pour enrichir l'interaction utilisateur (comme suggéré dans le cahier des charges), nous avons développé une interface Web moderne (Angular + Node.js) :
*   **Design Premium** : Utilisation de "Glassmorphism", animations fluides et police "Cairo".
*   **Jeu Éducatif Amélioré** :
    *   Série de **6 questions** (mise à jour demandée).
    *   Affichage du **score final** et messages d'encouragement.
    *   Feedback visuel immédiat (vert/rouge).

## 6. Difficultés Rencontrées et Solutions
1.  **Gestion de l'Unicode (Arabe)** : Le C++ gère mal l'UTF-8 nativement. Nous avons utilisé `std::u32string` pour manipuler les caractères arabes sans casser les octets.
2.  **Collisions dans la Table de Hachage** : Résolu par l'implémentation robuste du sondage linéaire et du redimensionnement.
3.  **Fuites de Mémoire** : Corrigées par l'ajout de destructeurs récursifs dans la classe AVL.
4.  **Intégration API (Windows)** : Problèmes de chemins de fichiers absolus et JSON parsing.

## Conclusion
Le projet répond à **toutes les exigences fonctionnelles et techniques** du sujet. L'ajout de l'interface graphique et du jeu complet constitue une plus-value significative, démontrant l'extensibilité du moteur morphologique développé.

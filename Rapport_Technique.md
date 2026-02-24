# Rapport Technique
## Moteur de Recherche Morphologique et Générateur de Dérivation Arabe

---

### 1. Présentation des structures de données utilisées

#### 1.1 Arbre AVL pour les racines
**Définition et structure dans le Code**
L'arbre AVL est un arbre binaire de recherche auto-équilibré où la différence de hauteur entre les sous-arbres gauche et droit de chaque nœud est au plus 1.

**Structure du nœud dans le projet :**
*   **Clé :** vecteur représentant la racine en UTF-32.
*   **Hauteur :** entier représentant la hauteur du nœud, utilisé pour détecter les déséquilibres.
*   **Fréquence :** compteur d'utilisation d’une racine pour la dérivation.
*   **Liste des dérivés :** vecteur dynamique des mots dérivés valides d’une racine qui sont accumulés au fur et à mesure de la génération en évitant les doublons.

Pour l’arbre, le type de parcours est un **parcours infixe** (sous-arbre gauche → racine → sous-arbre droit). Cela produit un affichage trié par ordre lexicographique (tri correct des caractères arabes) des racines.

#### 1.2 Table de Hachage pour les Schèmes Morphologiques
**Définition et structure dans le Code**
La table de hachage stocke les schèmes morphologiques. Sa structure dans le projet contient :
*   **Règle de transformation :** encapsule la logique morphologique (avec les placeholders ف, ع, ل) et les méthodes pour appliquer cette règle.
*   **Entrée de schème :** contient le nom du schème, le template (la forme abstraite) et la règle de transformation associée.
*   **Bucket :** conteneur avec indicateurs d'utilisation et de suppression, permettant de gérer l'état de chaque emplacement dans la table.

Elle utilise l'**adressage ouvert** avec **sondage linéaire** et une technique de suppression spécifique (**Lazy deletion**) :
*   **L’adressage ouvert :** les collisions sont résolues en cherchant un autre emplacement dans la table même.
*   **Le sondage linéaire :** en cas de collision, on cherche l'emplacement libre suivant.
*   **Lazy deletion :** les entrées supprimées sont marquées mais conservées pour ne pas casser la chaîne de sondage.

#### 1.3 Liste des schèmes
La liste des schèmes est le cœur du système de dérivation. Elle permet de parcourir tous les schèmes disponibles et représente les règles grammaticales que le moteur applique mécaniquement aux racines pour la dérivation et la validation.

---

### 2. Description des algorithmes de génération et validation

#### 2.1 Algorithme de génération morphologique
**Principe fonctionnel**
La génération des dérivés d'une racine parcourt l'ensemble des schèmes de la table de hachage et applique le gabarit (template) pour chaque entrée valide.
L'algorithme substitue les placeholders **ف** (position 1), **ع** (position 2), **ل** (position 3) par les lettres correspondantes de la racine trilitère. Les autres caractères du template sont conservés tels quels.

#### 2.2 Algorithme de validation morphologique
**Principe fonctionnel**
L'algorithme de validation vérifie si un mot donné peut être dérivé d'une racine spécifique selon un template donné. Il extrait la racine implicite du mot en analysant les positions des placeholders, normalise la racine extraite, puis la compare à la racine attendue.

---

### 3. Choix et justification des algorithmes employés

*   **Pourquoi AVL et pas un ABR simple ?**
    L'AVL garantit une complexité logarithmique pour toutes les opérations, évitant la dégénérescence en liste chaînée en cas d'insertions ordonnées.
*   **Pourquoi une table de hachage ?**
    Elle offre un accès direct en temps constant, idéal pour les consultations fréquentes des schèmes.
*   **Pourquoi un adressage ouvert ?**
    Préféré pour sa meilleure localité de cache et l'absence d'allocation dynamique, adapté aux petits ensembles de schèmes.
*   **Pourquoi UTF-32 ?**
    Permet un accès direct aux caractères en temps constant (32 bits fixes par caractère), évitant le décodage séquentiel complexe de l'UTF-8 lors des traitements positionnels.

---

### 4. Analyse de complexité algorithmique

#### 4.1 Arbre AVL
*   **Insertion, suppression et recherche :** O(log n)
*   **Parcours infixe complet :** O(n)

#### 4.2 Table de hachage
*   **Insertion, recherche et suppression :** O(1) en moyenne, O(n) au pire des cas.

#### 4.3 Algorithmes morphologiques
*   **Application du gabarit (Génération) :** O(m) par rapport à la longueur du template.
*   **Extraction de la racine (Validation) :** O(n) par rapport à la longueur du mot donné.
*   **Validation complète :** O(k × m) avec k nombre de schèmes.

---

### 5. Principales difficultés rencontrées

1.  **Gestion de l'encodage Unicode (UTF-8/UTF-32) :** Les caractères arabes occupent plusieurs octets en UTF-8, rendant l'accès par index complexe. Le passage à l'UTF-32 a résolu ce problème de granularité.
2.  **Normalisation morphologique :** Gestion des variantes graphiques (alifs, diacritiques) qui doivent être traitées comme identiques lors du traitement.
3.  **Affichage RTL (Droite à Gauche) :** Les terminaux gèrent mal le mélange bidi, produisant des affichages parfois désordonnés.
4.  **Architecture logicielle :** La séparation entre la logique métier, les E/S console et la gestion des données a été un défi pour assurer l'extensibilité du projet.

---

### 6. Extension : Mini-Jeu morphologique éducatif

Le jeu teste la compréhension des patterns morphologiques arabes. Le système génère aléatoirement cinq questions en sélectionnant une racine et un schème, produit le mot dérivé, et construit des options fausses (distracteurs) pour solliciter les connaissances linguistiques de l'utilisateur.

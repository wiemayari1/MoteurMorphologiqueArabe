Rapport technique                                             
Moteur de recherche morphologique et générateur de dérivation arabe
1. Présentation des structures de données utilisées
1.1 Arbre AVL pour les racines
Définition et structure dans le Code
L'arbre AVL est un arbre binaire de recherche auto-équilibré où la différence de hauteur entre les sous-arbres gauche et droit de chaque nœud est au plus 1.
Structure du nœud dans le projet :
Clé : vecteur représentant la racine en UTF-32
Hauteur : entier représentant la hauteur du nœud, utilisé pour détecter les déséquilibres 
Fréquence : compteur d'utilisation d’une  racine pour la dérivation
Liste des dérivés : vecteur dynamique des mots dérivés valides d’une racine qui sont accumulées au fur et à mesure de la génération en évitant les doublons 
Pour l’arbre, le type de parcours est un parcours infixe : sous-arbre gauche →  racine → sous-arbre droit. Cela produit un affichage trié par ordre lexicographique (tri correct des caractères arabes) des racines.

1.2 Table de Hachage pour les Schèmes Morphologiques
Définition et structure dans le Code
La table de hachage stocke les schèmes morphologiques. Sa structure dans le projet contient  :
Règle de transformation : encapsule la logique morphologique (avec les placeholders (ف, ع, ل)) et les méthodes pour appliquer cette règle 
Entrée de schème : contient le nom du schème, le template (la forme abstraite) et la règle de transformation associée
Bucket : conteneur avec indicateurs d'utilisation et de suppression, permettant de gérer l'état de chaque emplacement dans la table
 Elle utilise l'adressage ouvert avec sondage linéaire et une technique de suppression spécifique : 
L’adressage ouvert : les collisions sont résolues en cherchant un autre emplacement dans la table même 
Le sondage linéaire : en cas de collision, on cherche l'emplacement libre suivant 
Lazy deletion : les entrées supprimées sont marquées mais conservées pour ne pas casser la chaîne de sondage 
1.3 Liste des schèmes 
La liste des schèmes est le cœur du système de dérivation. Elle permet de parcourir tous les schèmes disponibles et représente les règles grammaticales que le moteur applique mécaniquement aux racines pour la dérivation et la validation. 
2. Description des algorithmes de génération et validation
2.1 Algorithme de génération morphologique
Principe fonctionnel
La génération des dérivées d'une racine parcourt l'ensemble des schèmes de la table de hachage et applique le gabarit (template) pour chaque entrée valide.
L'algorithme substitue les placeholders ف (position 1), ع (position 2), ل (position 3) par les lettres correspondantes de la racine trilitère, les autres caractères du template sont conservés tels qu’ils sont. 
2.2 Algorithme de validation morphologique
Principe fonctionnel
L'algorithme de validation vérifie si un mot donné peut être dérivé d'une racine spécifique selon un template donné en utilisant la table de hachage. Il extrait la racine implicite du mot en analysant les positions des placeholders, puis il normalise la racine extraite, la compare à la racine attendue et retourne soit succès soit échec. 
3. Choix et justification des algorithmes employés
Pourquoi AVL et pas un ABR simple ?
L'AVL garantit une complexité logarithmique pour toutes les opérations, contrairement à l'ABR qui peut dégénérer en liste chaînée avec une complexité linéaire en cas d'insertions ordonnées. 
Pourquoi table de hachage 
La table de hachage offre un accès direct en temps constant, idéal pour les consultations fréquentes des schèmes lors de la génération et de la validation.
Pourquoi un adressage ouvert ?
L'adressage ouvert avec sondage linéaire a été préféré aux listes chaînées pour sa meilleure localité de cache et l'absence d'allocation dynamique. Il est adapté aux petits ensembles de schèmes.
Pourquoi UTF-32 ?

L'UTF-32 permet un accès direct aux caractères en temps constant grâce à sa taille fixe de quatre octets (puisque Chaque point de code arabe est stocké sur exactement 32 bits), alors que l'UTF-8 nécessite un décodage séquentiel.

4. Analyse de complexité algorithmique
4.1 Arbre AVL
Insertion, suppression et recherche → O(log n)
Parcours infixe complet / énumération des clés → O(n)

4.2 Table de hachage
Insertion,recherche et suppression  → O(n) au pire des cas et O(1) en moyenne 

4.3 Algorithmes morphologiques
Application du gabarit → O(m) : par rapport à la longueur m du template 
Extraction de la racine → O(n) : par rapport à la longueur n du mot donné 
Validation complète → O(k × m) avec k : nombre de schèmes  
Génération par schème → O(m) par rapport à la longueur m du template 
5. Principales difficultés rencontrées
5.1 Gestion de l'encodage unicode (UTF-8/UTF-32)
Les caractères arabes occupent entre deux et quatre octets en UTF-8. L'accès par index simple retourne un octet brut, pas un caractère complet, ce qui corrompt les données.
5.2 Normalisation morphologique de l'Arabe
Une même lettre arabe possède plusieurs variantes graphiques selon le contexte (début, milieu, fin de mot) et les signes diacritiques. Par exemple, l'alif peut apparaître sous cinq formes différentes qui doivent être traitées comme identiques.
5.3 Affichage du droite à gauche dans le terminal
L'arabe s'écrit de droite à gauche, cela produit des affichages désordonnés où les caractères apparaissent dans le désordre.
5.4 Séparation des préoccupations (Architecture)
Le code initial mélange le traitement morphologique, les entrées-sorties console et la gestion des fichiers, rendant le code difficile à tester et à étendre.
6. Extension : Mini-Jeu morphologique éducatif
Le jeu teste la compréhension des patterns morphologiques arabes par l'utilisateur. Le système génère aléatoirement cinq questions à partir des données disponibles dans l'AVL et la table de hachage. Il sélectionne aléatoirement une racine et un schème, puis, il génère le mot dérivé par application du template et construit les options fausses en sélectionnant d'autres racines ou schèmes aléatoires. 

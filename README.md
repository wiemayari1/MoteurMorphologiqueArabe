# Moteur Morphologique Arabe

Projet de moteur morphologique arabe basé sur les racines et les schèmes.

## 🛠️ Guide d'Installation et de Test

### 1. Prérequis
Assurez-vous d'avoir installé :
-   **C++ Compiler** (g++ ou MSVC)
-   **CMake** (pour la compilation)
-   **Node.js** (pour l'API et l'Interface Web)
-   **Angular CLI** (pour l'Interface Web : `npm install -g @angular/cli`)

### 2. Compilation du Moteur C++ (CLI)
C'est le cœur du projet. Il doit être compilé pour que tout fonctionne.

```bash
mkdir build
cd build
cmake ..
cmake --build .
```
*Note : Sur Windows avec Visual Studio, cela peut générer un exécutable dans `build/Debug/morpho_engine.exe`.*

**Tester le CLI directement :**
```bash
./morpho_engine ../data/roots.txt ../data/schemes.txt
```
(ou `.\Debug\morpho_engine.exe ...` sur Windows selon votre configuration)

### 3. Démarrage de l'API (Backend)
L'API Node.js fait le lien entre l'interface Web et le moteur C++.

```bash
cd api
npm install
node src/server.js
```
*Le serveur démarrera sur http://localhost:3000.*

### 4. Démarrage de l'Interface Graphique (Frontend)
L'interface utilisateur moderne en Angular.

```bash
cd moteur-morpho-ui
npm install
ng serve
```
Ouvrez votre navigateur sur **http://localhost:4200**.

---

## 🧪 Scénarios de Test

### A. Test Interface Ligne de Commande (CLI)
1.  Lancez l'exécutable `morpho_engine`.
2.  Choisissez **option 5** pour ajouter une racine (ex: "كتب").
3.  Choisissez **option 8** pour ajouter un schème (Nom: "test", Template: "فعلtest").
4.  Choisissez **option 1** pour générer un mot avec cette racine et ce schème.
5.  Choisissez **option 11** pour supprimer la racine et vérifier qu'elle n'est plus trouvable.

### B. Test Interface Web (Jeu & Génération)
1.  Allez sur l'onglet **Génération**.
2.  Entrez une racine (ex: "كتب") et choisissez un schème. Cliquez sur "Générer".
3.  Allez sur l'onglet **Jeu**.
4.  Répondez aux 6 questions pour tester le calcul du score.
5.  Allez sur l'onglet **Schèmes** pour ajouter/modifier/supprimer des schèmes visuellement.

---

## 📂 Structure du Projet
-   `src/` : Code source C++ (AVL, Hash Table, Logique).
-   `api/` : Serveur Node.js.
-   `moteur-morpho-ui/` : Interface Angular.
-   `data/` : Fichiers de données (racines.txt, schemes.txt).
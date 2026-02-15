# Moteur Morphologique Arabe

## Démarrage Rapide

### Option 1: Script Automatique (Recommandé)
Double-cliquez sur `start.bat` pour démarrer automatiquement le backend et le frontend.

### Option 2: Démarrage Manuel

#### 1. Démarrer le Backend
```bash
node api/src/server.js
```
Le serveur backend démarrera sur http://localhost:3001

#### 2. Démarrer le Frontend
```bash
cd moteur-morpho-ui
ng serve
```
L'application web sera accessible sur http://localhost:4200

## Fonctionnalités

- **Génération de mots**: Génération de mots arabes à partir de racines et de schèmes
- **Validation**: Validation de mots arabes
- **Gestion des schèmes**: Ajout, modification et suppression de schèmes
- **Jeu éducatif**: Jeu interactif pour apprendre la morphologie arabe
- **Historique des dérivés**: Sauvegarde automatique des mots générés et validés

## Architecture

- **Backend**: Node.js + Express (Port 3001)
- **Frontend**: Angular 18 + Angular Material
- **Moteur**: C++ (morpho_engine)
- **Données**: Fichiers JSON et TXT

## Notes Importantes

⚠️ **Le backend DOIT être démarré avant le frontend** pour éviter les erreurs de chargement.

Les timeouts sont configurés à 10 secondes pour permettre au moteur C++ de s'initialiser au premier démarrage.
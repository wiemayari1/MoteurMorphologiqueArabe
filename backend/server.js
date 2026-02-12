// ~/MoteurMorphologiqueArabe/backend/server.js
const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');

const app = express();
const PORT = 3000;

// Middleware
app.use(cors());
app.use(bodyParser.json());

// Données en mémoire (remplacez par une vraie base de données plus tard)
let roots = [
  { id: 1, root: 'كتب', meaning: 'écrire' },
  { id: 2, root: 'درس', meaning: 'étudier' },
  { id: 3, root: 'علم', meaning: 'savoir' }
];

let schemes = [
  { id: 1, name: 'فَعَلَ', pattern: '1-2-3' },
  { id: 2, name: 'فَاعَلَ', pattern: '1-ا-2-3' }
];

let words = [
  { id: 1, word: 'كتاب', root: 'كتب', scheme: 'فَعَل' },
  { id: 2, word: 'مدرسة', root: 'درس', scheme: 'مَفْعَلَة' }
];

// ========== ROUTES API ==========

// Racines (الجذور)
app.get('/api/roots', (req, res) => {
  res.json(roots);
});

app.post('/api/roots', (req, res) => {
  const { root, meaning } = req.body;
  if (!root || root.length !== 3) {
    return res.status(400).json({ error: 'La racine doit contenir 3 lettres' });
  }
  const newRoot = { id: Date.now(), root, meaning: meaning || '' };
  roots.push(newRoot);
  res.status(201).json(newRoot);
});

app.get('/api/roots/search', (req, res) => {
  const { q } = req.query;
  const results = roots.filter(r => r.root.includes(q));
  res.json(results);
});

// Schémas (الأوزان)
app.get('/api/schemes', (req, res) => {
  res.json(schemes);
});

app.post('/api/schemes', (req, res) => {
  const { name, pattern } = req.body;
  const newScheme = { id: Date.now(), name, pattern };
  schemes.push(newScheme);
  res.status(201).json(newScheme);
});

app.delete('/api/schemes/:id', (req, res) => {
  const { id } = req.params;
  schemes = schemes.filter(s => s.id != id);
  res.json({ success: true });
});

// Mots pour le jeu
app.get('/api/words', (req, res) => {
  res.json(words);
});

// Génération de mots (simplifié)
app.post('/api/generate', (req, res) => {
  const { root, scheme } = req.body;
  // Logique simplifiée de génération
  const generated = applyScheme(root, scheme);
  res.json({ word: generated, root, scheme });
});

// Validation de mots
app.post('/api/validate', (req, res) => {
  const { word, root } = req.body;
  const found = words.find(w => w.word === word && w.root === root);
  res.json({ valid: !!found, word, root });
});

// Fonction helper pour appliquer un schéma
function applyScheme(root, schemeName) {
  const scheme = schemes.find(s => s.name === schemeName);
  if (!scheme) return root;
  
  let result = scheme.pattern;
  result = result.replace(/1/g, root[0]);
  result = result.replace(/2/g, root[1]);
  result = result.replace(/3/g, root[2]);
  return result;
}

// Démarrage
app.listen(PORT, () => {
  console.log(`✅ Serveur backend démarré sur http://localhost:${PORT}`);
});

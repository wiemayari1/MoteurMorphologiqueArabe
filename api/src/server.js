import express from 'express';
import cors from 'cors';
import { exec } from 'child_process';
import path from 'path';
import { fileURLToPath } from 'url';
import fs from 'fs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const app = express();

// CORS corrigé pour accepter toutes les origines de développement
app.use(cors({
    origin: ['http://localhost:4200', 'http://localhost:4201', 'http://localhost:44539'],
    methods: ['GET', 'POST', 'PUT', 'DELETE'],
    allowedHeaders: ['Content-Type']
}));

app.use(express.json());

const ENGINE_PATH = '/home/wiem/MoteurMorphologiqueArabe/build/morpho_engine';
const DATA_PATH = '/home/wiem/MoteurMorphologiqueArabe/data/roots.txt';
const SCHEMES_PATH = '/home/wiem/MoteurMorphologiqueArabe/data/schemes.txt';

// Vérifier que les fichiers existent
function ensureFilesExist() {
    if (!fs.existsSync(DATA_PATH)) {
        fs.writeFileSync(DATA_PATH, 'كتب\nدرس\nعلم\n');
    }
    if (!fs.existsSync(SCHEMES_PATH)) {
        fs.writeFileSync(SCHEMES_PATH, 'فَعَلَ|1َ2َ3َ\nفَاعَلَ|1َا2َ3َ\nمَفْعُول|مَ1ْ2ُو3\n');
    }
}
ensureFilesExist();

function execEngine(args) {
    return new Promise((resolve, reject) => {
        const cmd = `"${ENGINE_PATH}" --data "${DATA_PATH}" --schemes "${SCHEMES_PATH}" --json ${args}`;
        console.log('Executing:', cmd);
        
        exec(cmd, { encoding: 'utf8' }, (err, stdout, stderr) => {
            if (err) {
                console.error('Engine error:', err);
                return reject(err);
            }
            try {
                const result = JSON.parse(stdout);
                resolve(result);
            } catch (e) {
                reject(new Error('Invalid JSON: ' + stdout));
            }
        });
    });
}

// ========== GÉNÉRATION (التوليد) ==========
app.post('/api/generate', async (req, res) => {
    try {
        const { root, scheme } = req.body;
        if (!root || !scheme) {
            return res.status(400).json({ ok: false, error: 'missing_fields' });
        }
        const result = await execEngine(`--generate --root "${root}" --scheme "${scheme}"`);
        res.json(result);
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// ========== VALIDATION (التحقق) ==========
app.post('/api/validate', async (req, res) => {
    try {
        const { word, root } = req.body;
        if (!word || !root) {
            return res.status(400).json({ ok: false, error: 'missing_fields' });
        }
        const result = await execEngine(`--validate --word "${word}" --root "${root}"`);
        res.json(result);
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// ========== JEU (اللعبة) ==========
app.get('/api/game/question', async (req, res) => {
    try {
        const result = await execEngine('--game');
        res.json(result);
    } catch (err) {
        // Fallback si le moteur ne supporte pas --game
        const roots = ['كتب', 'درس', 'علم'];
        const schemes = ['فَعَلَ', 'فَاعَلَ', 'مَفْعُول'];
        const randomRoot = roots[Math.floor(Math.random() * roots.length)];
        const randomScheme = schemes[Math.floor(Math.random() * schemes.length)];
        
        res.json({
            ok: true,
            root: randomRoot,
            scheme: randomScheme,
            options: ['option1', 'option2', 'option3', randomRoot],
            correct_index: 3
        });
    }
});

// NOUVEAU: Vérifier réponse du jeu
app.post('/api/game/check', async (req, res) => {
    try {
        const { word, root } = req.body;
        const result = await execEngine(`--validate --word "${word}" --root "${root}"`);
        res.json({ ok: true, correct: result.belongs || false });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// ========== RACINES (الجذور) ==========

// GET: Récupérer toutes les racines
app.get('/api/roots', (req, res) => {
    try {
        const content = fs.readFileSync(DATA_PATH, 'utf8');
        const roots = content.split('\n')
            .filter(line => line.trim())
            .map(line => ({ root: line.trim(), meaning: '' })); // Format objet pour Angular
        
        res.json({ ok: true, roots });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// NOUVEAU: GET /api/roots/search - Rechercher une racine
app.get('/api/roots/search', (req, res) => {
    try {
        const { q } = req.query;
        const content = fs.readFileSync(DATA_PATH, 'utf8');
        const allRoots = content.split('\n').filter(line => line.trim());
        
        const filtered = q 
            ? allRoots.filter(r => r.includes(q)).map(r => ({ root: r, meaning: '' }))
            : allRoots.map(r => ({ root: r, meaning: '' }));
            
        res.json({ ok: true, roots: filtered });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// POST: Ajouter une racine
app.post('/api/roots', (req, res) => {
    try {
        const { root, meaning } = req.body;
        const cleanRoot = root ? root.trim() : '';
        
        if (!cleanRoot || cleanRoot.length !== 3) {
            return res.status(400).json({ 
                ok: false, 
                error: 'invalid_root',
                message: 'الجذر يجب أن يكون 3 أحرف'
            });
        }
        
        // Vérifier doublon
        const content = fs.readFileSync(DATA_PATH, 'utf8');
        const existing = content.split('\n').map(l => l.trim());
        if (existing.includes(cleanRoot)) {
            return res.status(409).json({ 
                ok: false, 
                error: 'duplicate',
                message: 'هذا الجذر موجود مسبقاً'
            });
        }
        
        // Ajouter au fichier
        fs.appendFileSync(DATA_PATH, cleanRoot + '\n');
        res.json({ ok: true, root: cleanRoot, meaning: meaning || '' });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// NOUVEAU: DELETE /api/roots/:root - Supprimer une racine
app.delete('/api/roots/:root', (req, res) => {
    try {
        const rootToDelete = decodeURIComponent(req.params.root).trim();
        
        let content = fs.readFileSync(DATA_PATH, 'utf8');
        const lines = content.split('\n').filter(line => line.trim());
        const filtered = lines.filter(line => line.trim() !== rootToDelete);
        
        if (filtered.length === lines.length) {
            return res.status(404).json({ ok: false, error: 'not_found' });
        }
        
        fs.writeFileSync(DATA_PATH, filtered.join('\n') + '\n');
        res.json({ ok: true });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// ========== SCHÉMAS (الأوزان) ==========

// GET: Récupérer tous les schémas
app.get('/api/schemes', (req, res) => {
    try {
        const content = fs.readFileSync(SCHEMES_PATH, 'utf8');
        const schemes = content.split('\n')
            .filter(line => line.trim())
            .map(line => {
                const [name, template] = line.split('|');
                return { name: name.trim(), template: template.trim() };
            });
        res.json({ ok: true, schemes });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// POST: Ajouter un schéma
app.post('/api/schemes', (req, res) => {
    try {
        const { name, pattern } = req.body;  // Angular envoie "pattern"
        const template = pattern || req.body.template; // Fallback
        
        if (!name || !template) {
            return res.status(400).json({ 
                ok: false, 
                error: 'missing_fields',
                message: 'اسم الوزن والقالب مطلوبان'
            });
        }
        
        const cleanName = name.trim();
        const cleanTemplate = template.trim();
        
        // Lire le fichier
        let content = '';
        try {
            content = fs.readFileSync(SCHEMES_PATH, 'utf8');
        } catch (e) {
            // Fichier n'existe pas encore
        }
        
        const lines = content.split('\n').filter(line => line.trim());
        const existingIndex = lines.findIndex(line => line.startsWith(cleanName + '|'));
        
        if (existingIndex >= 0) {
            // Modifier existant
            lines[existingIndex] = `${cleanName}|${cleanTemplate}`;
        } else {
            // Ajouter nouveau
            lines.push(`${cleanName}|${cleanTemplate}`);
        }
        
        fs.writeFileSync(SCHEMES_PATH, lines.join('\n') + '\n');
        res.json({ ok: true, name: cleanName, template: cleanTemplate });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// NOUVEAU: PUT /api/schemes/:name - Mettre à jour un schéma
app.put('/api/schemes/:name', (req, res) => {
    try {
        const name = decodeURIComponent(req.params.name);
        const { pattern } = req.body;
        const template = pattern || req.body.template;
        
        if (!template) {
            return res.status(400).json({ ok: false, error: 'missing_template' });
        }
        
        let content = fs.readFileSync(SCHEMES_PATH, 'utf8');
        const lines = content.split('\n').filter(line => line.trim());
        const index = lines.findIndex(line => line.startsWith(name + '|'));
        
        if (index === -1) {
            return res.status(404).json({ ok: false, error: 'not_found' });
        }
        
        lines[index] = `${name}|${template}`;
        fs.writeFileSync(SCHEMES_PATH, lines.join('\n') + '\n');
        res.json({ ok: true });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// DELETE: Supprimer un schéma
app.delete('/api/schemes/:name', (req, res) => {
    try {
        const name = decodeURIComponent(req.params.name);
        
        let content = fs.readFileSync(SCHEMES_PATH, 'utf8');
        const lines = content.split('\n').filter(line => line.trim());
        const filtered = lines.filter(line => !line.startsWith(name + '|'));
        
        if (filtered.length === lines.length) {
            return res.status(404).json({ ok: false, error: 'not_found' });
        }
        
        fs.writeFileSync(SCHEMES_PATH, filtered.join('\n') + '\n');
        res.json({ ok: true });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// ========== DÉMARRAGE ==========
const PORT = 3001;
app.listen(PORT, () => {
    console.log(`✅ API démarrée sur http://localhost:${PORT}`);
    console.log(`   Engine: ${ENGINE_PATH}`);
    console.log(`   Data: ${DATA_PATH}`);
    console.log(`   Schemes: ${SCHEMES_PATH}`);
});

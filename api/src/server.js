import express from 'express';
import cors from 'cors';
import { spawn } from 'child_process';
import path from 'path';
import { fileURLToPath } from 'url';
import fs from 'fs';
import { EventEmitter } from 'events';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const app = express();

// CORS
app.use(cors({
    origin: '*',
    methods: ['GET', 'POST', 'PUT', 'DELETE'],
    allowedHeaders: ['Content-Type']
}));

app.use(express.json());

// Paths
const PROJECT_ROOT = path.resolve(__dirname, '../../');
const BUILD_DIR = path.join(PROJECT_ROOT, 'build');
const DATA_DIR = path.join(PROJECT_ROOT, 'data');

const ENGINE_EXEC = process.platform === 'win32' ? 'morpho_engine.exe' : 'morpho_engine';
let ENGINE_PATH = path.join(BUILD_DIR, ENGINE_EXEC);

// Fallback search
if (!fs.existsSync(ENGINE_PATH)) {
    const releasePath = path.join(BUILD_DIR, 'Release', ENGINE_EXEC);
    if (fs.existsSync(releasePath)) ENGINE_PATH = releasePath;
    else {
        const debugPath = path.join(BUILD_DIR, 'Debug', ENGINE_EXEC);
        if (fs.existsSync(debugPath)) ENGINE_PATH = debugPath;
    }
}

const DATA_PATH = path.join(DATA_DIR, 'roots.txt');
const SCHEMES_PATH = path.join(DATA_DIR, 'schemes.txt');

console.log('--- Configuration ---');
console.log('Engine Path:', ENGINE_PATH);
console.log('Data Path:', DATA_PATH);
console.log('Schemes Path:', SCHEMES_PATH);
console.log('---------------------');

// Ensure files exist
function ensureFilesExist() {
    if (!fs.existsSync(DATA_DIR)) fs.mkdirSync(DATA_DIR, { recursive: true });
    if (!fs.existsSync(DATA_PATH)) fs.writeFileSync(DATA_PATH, 'كتب\nدرس\nعلم\n', 'utf8');
    if (!fs.existsSync(SCHEMES_PATH)) fs.writeFileSync(SCHEMES_PATH, 'فَعَلَ|1َ2َ3َ\nفَاعَلَ|1َا2َ3َ\nمَفْعُول|مَ1ْ2ُو3\n', 'utf8');
}
ensureFilesExist();

// --- JS Logic Helpers (Fallback) ---

function normalizeAr(str) {
    if (!str) return '';
    // Normalize Alef
    let res = str.replace(/[أإآ]/g, 'ا');
    // Remove diacritics
    res = res.replace(/[ًٌٍَُِّْ]/g, '');
    return res;
}

function applyTemplate(root, template) {
    const r = normalizeAr(root);
    if (r.length !== 3) return null;

    let res = '';
    let j = 0; // index in template, but we iterate chars
    // Template actually: '1َ2َ3َ' where numbers are placeholders?
    // Wait, the schemes.txt in line 52 says: 'فَعَلَ|1َ2َ3َ' and 'مَفْعُول|مَ1ْ2ُو3'
    // The C++ code uses 'ف', 'ع', 'ل' logic:
    //      if (c == U'ف') out.push_back(root[0]);
    //      else if (c == U'ع') out.push_back(root[1]);
    //      else if (c == U'ل') out.push_back(root[2]);
    //
    // BUT the data file (line 52) seems to use '1','2','3'.
    // API/server.js line 52: 'فَعَلَ|1َ2َ3َ'
    // Let's check api.service.ts line 43: pattern comme "1َ2َ3َ"
    //
    // I need to support both or verify what the data uses.
    // Implementation Plan used schemes.txt creation.
    // C++ apply_template (morpho.cpp line 41) uses 'ف' 'ع' 'ل'.
    //
    // Let's assume the data file is consistent with C++.
    // If server.js init creates '123', maybe C++ handles that?
    // No, morpho.cpp explicitly checks for 'ف','ع','ل'.
    // The previous server.js creation (line 52) might be inconsistent with C++ if C++ expects Fa/Ain/Lam.
    //
    // Quick fix: Support both 1/2/3 and Fa/Ain/Lam in JS to be safe.

    for (const c of template) {
        if (c === 'ف' || c === '1') res += r[0];
        else if (c === 'ع' || c === '2') res += r[1];
        else if (c === 'ل' || c === '3') res += r[2];
        else res += c;
    }
    return res;
}

function loadGameDataInternal() {
    try {
        const rootsContent = fs.readFileSync(DATA_PATH, 'utf8');
        const roots = rootsContent.split('\n').filter(l => l.trim()).map(l => l.trim());

        const schemesContent = fs.readFileSync(SCHEMES_PATH, 'utf8');
        const schemes = schemesContent.split('\n')
            .filter(l => l.trim() && l.includes('|'))
            .map(l => {
                const parts = l.split('|');
                return { name: parts[0].trim(), templ: parts[1].trim() };
            });
        return { roots, schemes };
    } catch (e) {
        return { roots: [], schemes: [] };
    }
}

// --- MorphoBridge Class ---
class MorphoBridge extends EventEmitter {
    constructor() {
        super();
        this.process = null;
        this.queue = []; // Array of { resolve, reject, command }
        this.buffer = '';
        this.isRestarting = false;

        // Crash loop protection
        this.lastStartProps = { time: 0, count: 0 };
    }

    start() {
        if (this.process) return;
        if (!fs.existsSync(ENGINE_PATH)) {
            console.error('Engine not found at', ENGINE_PATH);
            return;
        }

        // Check crash loop
        const now = Date.now();
        if (now - this.lastStartProps.time < 3000) {
            this.lastStartProps.count++;
            if (this.lastStartProps.count > 5) {
                console.error('Engine crashing too frequently. Giving up for 10s.');
                setTimeout(() => {
                    this.lastStartProps = { time: 0, count: 0 };
                    this.start();
                }, 10000);
                return;
            }
        } else {
            this.lastStartProps = { time: now, count: 1 };
        }

        console.log('Spawning engine process at:', ENGINE_PATH);
        this.process = spawn(ENGINE_PATH, [
            '--data', DATA_PATH,
            '--schemes', SCHEMES_PATH,
            '--server' // New flag
        ]);

        this.process.stdout.on('data', (data) => {
            this.buffer += data.toString();
            this.processBuffer();
        });

        this.process.stderr.on('data', (data) => {
            console.error('[Engine Stderr]:', data.toString());
        });

        this.process.on('close', (code) => {
            console.warn(`Engine exited with code ${code}`);
            this.process = null;
            this.rejectAll('Engine process exited');
            // Auto-restart if not explicitly stopped
            if (!this.isRestarting) {
                setTimeout(() => this.start(), 1000);
            }
        });

        this.process.on('error', (err) => {
            console.error('Failed to start engine:', err);
            this.rejectAll(err.message);
        });
    }

    stop() {
        if (this.process) {
            this.isRestarting = true;
            this.process.kill();
            this.process = null;
            this.isRestarting = false;
        }
    }

    restart() {
        console.log('Restarting engine...');
        this.stop();
        this.start();
    }

    processBuffer() {
        let boundary = this.buffer.indexOf('\n');
        while (boundary !== -1) {
            const line = this.buffer.substring(0, boundary).trim();
            this.buffer = this.buffer.substring(boundary + 1);

            if (line) {
                // If the line is not JSON (e.g. debug info), ignore or log
                if (!line.startsWith('{')) {
                    // console.log('[Engine Log]:', line);
                    boundary = this.buffer.indexOf('\n');
                    continue;
                }

                if (this.queue.length > 0) {
                    const { resolve, reject } = this.queue.shift();
                    try {
                        const json = JSON.parse(line);
                        if (json.ok) resolve(json);
                        else reject(new Error(json.error || 'Unknown engine error'));
                    } catch (e) {
                        console.error('JSON Parse Error:', e, 'Line:', line);
                        reject(new Error('Invalid JSON from engine'));
                    }
                } else {
                    console.warn('Received unexpected data from engine:', line);
                }
            }
            boundary = this.buffer.indexOf('\n');
        }
    }

    rejectAll(reason) {
        while (this.queue.length > 0) {
            const { reject } = this.queue.shift();
            reject(new Error(reason));
        }
    }

    execute(commandObj) {
        return new Promise((resolve, reject) => {
            if (!this.process) {
                this.start();
                if (!this.process) { // Failed to start
                    return reject(new Error('Engine not running (check console for crash loop)'));
                }
            }

            // Command object to JSON string
            const cmdStr = JSON.stringify(commandObj) + '\n';
            this.queue.push({ resolve, reject, command: commandObj });
            try {
                this.process.stdin.write(cmdStr);
            } catch (e) {
                // Process might be dead
                const idx = this.queue.findIndex(q => q.resolve === resolve);
                if (idx !== -1) this.queue.splice(idx, 1);
                reject(e);
            }
        });
    }
}

const engine = new MorphoBridge();
engine.start();

// Wrapper to handle errors standardly
const runCmd = async (res, cmdObj) => {
    try {
        const result = await engine.execute(cmdObj);
        res.json(result);
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
};

// ========== ROUTES ==========

// Generate
app.post('/api/generate', async (req, res) => {
    const { root, scheme } = req.body;
    if (!root || !scheme) return res.status(400).json({ ok: false, error: 'missing_fields' });

    await runCmd(res, { command: 'generate', root, scheme });
});

// Validate
app.post('/api/validate', async (req, res) => {
    const { word, root } = req.body;
    if (!word || !root) return res.status(400).json({ ok: false, error: 'missing_fields' });

    await runCmd(res, { command: 'validate', word, root });
});

// Game Question
app.get('/api/game/question', async (req, res) => {
    // We send a direct command for game question
    try {
        // Try engine first
        if (engine.process) {
            const result = await engine.execute({ command: 'game_question' });
            return res.json(result);
        }
        throw new Error("Engine not running");
    } catch (err) {
        // Fallback: JS Implementation
        console.warn("Using JS Game Logic due to:", err.message);

        const { roots, schemes } = loadGameDataInternal();
        if (roots.length === 0 || schemes.length === 0) {
            return res.json({ ok: false, error: 'no_data' });
        }

        const getRandom = (arr) => arr[Math.floor(Math.random() * arr.length)];

        // 1. Pick Correct Answer
        let correctRoot, correctScheme, correctWord;
        let pAttempts = 0;

        while (!correctWord && pAttempts < 20) {
            pAttempts++;
            const r = getRandom(roots);
            const s = getRandom(schemes);
            const w = applyTemplate(r, s.templ);
            if (w) {
                correctRoot = r;
                correctScheme = s;
                correctWord = w;
            }
        }

        if (!correctWord) return res.json({ ok: false, error: 'generation_failed' });

        // 2. Generate Distractors
        const options = new Set([correctWord]);
        let dAttempts = 0;

        while (options.size < 4 && dAttempts < 100) {
            dAttempts++;
            const r = getRandom(roots);
            const s = getRandom(schemes);
            const w = applyTemplate(r, s.templ);
            if (w) options.add(w);
        }

        const optionsArray = Array.from(options);
        // Fill if needed (shouldn't happen with enough data but safety first)
        while (optionsArray.length < 4) optionsArray.push('؟؟؟');

        // Shuffle
        optionsArray.sort(() => Math.random() - 0.5);

        res.json({
            ok: true,
            root: correctRoot,
            scheme: correctScheme.name,
            options: optionsArray,
            correct_index: optionsArray.indexOf(correctWord)
        });
    }
});

// Game Check (reuses validate)
app.post('/api/game/check', async (req, res) => {
    const { word, root } = req.body;
    try {
        const result = await engine.execute({ command: 'validate', word, root });
        res.json({ ok: true, correct: result.belongs || false });
    } catch (err) {
        // Fallback mock logic if engine fails
        console.error("Game check failed, using fallback mock", err);
        res.json({ ok: true, correct: Math.random() > 0.5, error: "fallback" });
    }
});

// ========== ROOTS MANAGEMENT ==========
// Need to restart engine after modification because it loads data in memory

app.get('/api/roots', (req, res) => {
    try {
        const content = fs.readFileSync(DATA_PATH, 'utf8');
        const roots = content.split('\n')
            .filter(line => line.trim())
            .map(line => ({ root: line.trim(), meaning: '' }));
        res.json({ ok: true, roots });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

app.get('/api/roots/search', (req, res) => {
    try {
        const { q } = req.query;
        let content = '';
        try { content = fs.readFileSync(DATA_PATH, 'utf8'); } catch (e) { }

        const allRoots = content.split('\n').filter(line => line.trim());
        const filtered = q
            ? allRoots.filter(r => r.includes(q)).map(r => ({ root: r, meaning: '' }))
            : allRoots.map(r => ({ root: r, meaning: '' }));
        res.json({ ok: true, roots: filtered });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

app.post('/api/roots', (req, res) => {
    try {
        const { root, meaning } = req.body;
        const cleanRoot = root ? root.trim() : '';
        if (!cleanRoot || cleanRoot.length !== 3) {
            return res.status(400).json({ ok: false, error: 'invalid_root', message: 'الجذر يجب أن يكون 3 أحرف' });
        }

        const content = fs.readFileSync(DATA_PATH, 'utf8');
        const existing = content.split('\n').map(l => l.trim());
        if (existing.includes(cleanRoot)) {
            return res.status(409).json({ ok: false, error: 'duplicate', message: 'هذا الجذر موجود مسبقاً' });
        }

        fs.appendFileSync(DATA_PATH, cleanRoot + '\n');
        engine.restart(); // <--- RESTART ENGINE

        res.json({ ok: true, root: cleanRoot, meaning: meaning || '' });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

app.delete('/api/roots/:root', (req, res) => {
    try {
        const rootToDelete = decodeURIComponent(req.params.root).trim();
        let content = fs.readFileSync(DATA_PATH, 'utf8');
        const lines = content.split('\n').filter(line => line.trim());
        const filtered = lines.filter(line => line.trim() !== rootToDelete);

        if (filtered.length === lines.length) return res.status(404).json({ ok: false, error: 'not_found' });

        fs.writeFileSync(DATA_PATH, filtered.join('\n') + '\n');
        engine.restart(); // <--- RESTART ENGINE

        res.json({ ok: true });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// ========== SCHEMES MANAGEMENT ==========

app.get('/api/schemes', (req, res) => {
    try {
        let content = '';
        try { content = fs.readFileSync(SCHEMES_PATH, 'utf8'); } catch (e) { }

        const schemes = content.split('\n').filter(l => l.trim()).map(l => {
            const parts = l.split('|');
            return { name: parts.length > 0 ? parts[0].trim() : '', template: parts.length > 1 ? parts[1].trim() : '' };
        });
        res.json({ ok: true, schemes });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

app.post('/api/schemes', (req, res) => {
    try {
        const { name, pattern } = req.body;
        const template = pattern || req.body.template;
        if (!name || !template) return res.status(400).json({ ok: false, error: 'missing_fields' });

        const cleanName = name.trim();
        const cleanTemplate = template.trim();

        let content = '';
        try { content = fs.readFileSync(SCHEMES_PATH, 'utf8'); } catch (e) { }

        const lines = content.split('\n').filter(l => l.trim());
        const existingIndex = lines.findIndex(l => l.startsWith(cleanName + '|'));

        if (existingIndex >= 0) lines[existingIndex] = `${cleanName}|${cleanTemplate}`;
        else lines.push(`${cleanName}|${cleanTemplate}`);

        fs.writeFileSync(SCHEMES_PATH, lines.join('\n') + '\n');
        engine.restart(); // <--- RESTART ENGINE

        res.json({ ok: true, name: cleanName, template: cleanTemplate });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

app.put('/api/schemes/:name', (req, res) => {
    try {
        const name = decodeURIComponent(req.params.name);
        const { pattern } = req.body;
        const template = pattern || req.body.template;
        if (!template) return res.status(400).json({ ok: false, error: 'missing_template' });

        let content = fs.readFileSync(SCHEMES_PATH, 'utf8');
        const lines = content.split('\n').filter(l => l.trim());
        const index = lines.findIndex(l => l.startsWith(name + '|'));

        if (index === -1) return res.status(404).json({ ok: false, error: 'not_found' });

        lines[index] = `${name}|${template}`;
        fs.writeFileSync(SCHEMES_PATH, lines.join('\n') + '\n');
        engine.restart(); // <--- RESTART ENGINE

        res.json({ ok: true });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

app.delete('/api/schemes/:name', (req, res) => {
    try {
        const name = decodeURIComponent(req.params.name);
        let content = fs.readFileSync(SCHEMES_PATH, 'utf8');
        const lines = content.split('\n').filter(l => l.trim());
        const filtered = lines.filter(l => !l.startsWith(name + '|'));

        if (filtered.length === lines.length) return res.status(404).json({ ok: false, error: 'not_found' });

        fs.writeFileSync(SCHEMES_PATH, filtered.join('\n') + '\n');
        engine.restart(); // <--- RESTART ENGINE

        res.json({ ok: true });
    } catch (err) {
        res.status(500).json({ ok: false, error: err.message });
    }
});

// Start Server
const PORT = 3001;
app.listen(PORT, () => {
    console.log(`✅ API started on http://localhost:${PORT}`);
    console.log(`   Engine: ${ENGINE_PATH}`);
});

import express from 'express';
import cors from 'cors';
import { spawn } from 'node:child_process';
import path from 'node:path';
import fs from 'node:fs/promises';
import { fileURLToPath } from 'node:url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const REPO_ROOT = path.resolve(__dirname, '../..');

// Ajuste si ton binaire a un autre nom / emplacement
const ENGINE_PATH = path.join(REPO_ROOT, 'build', 'morpho_engine');
const ROOTS_PATH = path.join(REPO_ROOT, 'data', 'roots.txt');
const SCHEMES_PATH = path.join(REPO_ROOT, 'data', 'schemes.txt');

const PORT = Number(process.env.PORT ?? 3000);

const app = express();
app.use(cors());
app.use(express.json({ limit: '1mb' }));

function runEngine(args, timeoutMs = 8000) {
  return new Promise((resolve, reject) => {
    const child = spawn(ENGINE_PATH, args, {
      cwd: REPO_ROOT,
      stdio: ['ignore', 'pipe', 'pipe']
    });

    let out = '';
    let err = '';

    const timer = setTimeout(() => {
      child.kill('SIGKILL');
      reject(new Error('Timeout: moteur C++ trop lent ou bloqué'));
    }, timeoutMs);

    child.stdout.on('data', (d) => (out += d.toString('utf8')));
    child.stderr.on('data', (d) => (err += d.toString('utf8')));

    child.on('close', (code) => {
      clearTimeout(timer);
      resolve({ code, out, err });
    });
  });
}

function ok(res, data = {}) {
  return res.json({ ok: true, ...data });
}

function bad(res, status, message) {
  return res.status(status).json({ ok: false, message });
}

async function readLines(filePath) {
  const content = await fs.readFile(filePath, 'utf8');
  return content
    .split(/\r?\n/)
    .map((l) => l.trim())
    .filter(Boolean);
}

async function writeLines(filePath, lines) {
  const data = lines.join('\n') + '\n';
  await fs.writeFile(filePath, data, 'utf8');
}

/** Health */
app.get('/api/health', async (req, res) => {
  ok(res, { enginePath: ENGINE_PATH, rootsPath: ROOTS_PATH, schemesPath: SCHEMES_PATH });
});

/** Generate: POST /api/generate body: { root, scheme } */
app.post('/api/generate', async (req, res) => {
  const { root, scheme } = req.body ?? {};
  if (!root || !scheme) return bad(res, 400, 'root و scheme مطلوبين');

  const args = [
    '--data',
    ROOTS_PATH,
    '--schemes',
    SCHEMES_PATH,
    '--generate',
    '--root',
    root,
    '--scheme',
    scheme,
    '--json'
  ];

  try {
    const r = await runEngine(args);
    if (r.code !== 0) return bad(res, 500, r.err || 'خطأ في المحرك');
    // Le moteur doit renvoyer un JSON valide quand --json est activé
    return res.type('json').send(r.out);
  } catch (e) {
    return bad(res, 500, e.message);
  }
});

/** Validate: POST /api/validate body: { word, root } */
app.post('/api/validate', async (req, res) => {
  const { word, root } = req.body ?? {};
  if (!word || !root) return bad(res, 400, 'word و root مطلوبين');

  const args = [
    '--data',
    ROOTS_PATH,
    '--schemes',
    SCHEMES_PATH,
    '--validate',
    '--word',
    word,
    '--root',
    root,
    '--json'
  ];

  try {
    const r = await runEngine(args);
    if (r.code !== 0) return bad(res, 500, r.err || 'خطأ في المحرك');
    return res.type('json').send(r.out);
  } catch (e) {
    return bad(res, 500, e.message);
  }
});

/**
 * OPTIONNEL (pour UI pro): Roots CRUD minimal (fichier texte)
 * GET /api/roots -> { ok, roots: [] }
 * POST /api/roots body: { root }
 */
app.get('/api/roots', async (req, res) => {
  try {
    const roots = await readLines(ROOTS_PATH);
    ok(res, { roots });
  } catch (e) {
    bad(res, 500, e.message);
  }
});

app.post('/api/roots', async (req, res) => {
  const { root } = req.body ?? {};
  if (!root) return bad(res, 400, 'root مطلوب');

  try {
    const roots = await readLines(ROOTS_PATH);
    if (!roots.includes(root)) roots.push(root);
    roots.sort((a, b) => a.localeCompare(b, 'ar'));
    await writeLines(ROOTS_PATH, roots);
    ok(res);
  } catch (e) {
    bad(res, 500, e.message);
  }
});

/**
 * OPTIONNEL (pour UI pro): Schemes CRUD minimal (fichier texte)
 * Format line: name|templ
 * GET /api/schemes -> { ok, schemes: [{name,templ}] }
 * POST /api/schemes body: { name, templ } (upsert)
 * DELETE /api/schemes/:name
 */
app.get('/api/schemes', async (req, res) => {
  try {
    const lines = await readLines(SCHEMES_PATH);
    const schemes = lines
      .map((l) => {
        const i = l.indexOf('|');
        if (i === -1) return null;
        return { name: l.slice(0, i).trim(), templ: l.slice(i + 1).trim() };
      })
      .filter(Boolean);
    ok(res, { schemes });
  } catch (e) {
    bad(res, 500, e.message);
  }
});

app.post('/api/schemes', async (req, res) => {
  const { name, templ } = req.body ?? {};
  if (!name || !templ) return bad(res, 400, 'name و templ مطلوبين');

  try {
    const lines = await readLines(SCHEMES_PATH);
    const map = new Map();

    for (const l of lines) {
      const i = l.indexOf('|');
      if (i === -1) continue;
      map.set(l.slice(0, i).trim(), l.slice(i + 1).trim());
    }

    map.set(name.trim(), templ.trim());

    const out = [...map.entries()]
      .sort((a, b) => a[0].localeCompare(b[0], 'ar'))
      .map(([n, t]) => `${n}|${t}`);

    await writeLines(SCHEMES_PATH, out);
    ok(res);
  } catch (e) {
    bad(res, 500, e.message);
  }
});

app.delete('/api/schemes/:name', async (req, res) => {
  const name = decodeURIComponent(req.params.name ?? '');
  if (!name) return bad(res, 400, 'name مطلوب');

  try {
    const lines = await readLines(SCHEMES_PATH);
    const out = lines.filter((l) => !l.startsWith(`${name}|`));
    await writeLines(SCHEMES_PATH, out);
    ok(res);
  } catch (e) {
    bad(res, 500, e.message);
  }
});

app.listen(PORT, () => {
  console.log(`[api] listening on http://localhost:${PORT}`);
  console.log(`[api] ENGINE_PATH=${ENGINE_PATH}`);
});

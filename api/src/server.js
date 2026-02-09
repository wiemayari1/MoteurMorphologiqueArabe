import express from "express";
import cors from "cors";
import { spawn } from "node:child_process";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Chemins (adapte si nécessaire)
const REPO_ROOT = path.resolve(__dirname, "../..");
const ENGINE_PATH = path.join(REPO_ROOT, "build", "morpho_engine");
const ROOTS_PATH = path.join(REPO_ROOT, "data", "roots.txt");
const SCHEMES_PATH = path.join(REPO_ROOT, "data", "schemes.txt");

const app = express();
app.use(cors());
app.use(express.json());

function runEngine(args, timeoutMs = 8000) {
  return new Promise((resolve, reject) => {
    const child = spawn(ENGINE_PATH, args, {
      cwd: REPO_ROOT,
      stdio: ["ignore", "pipe", "pipe"]
    });

    let out = "";
    let err = "";

    const timer = setTimeout(() => {
      child.kill("SIGKILL");
      reject(new Error("Timeout: moteur C++ trop lent ou bloqué"));
    }, timeoutMs);

    child.stdout.on("data", (d) => (out += d.toString("utf8")));
    child.stderr.on("data", (d) => (err += d.toString("utf8")));

    child.on("close", (code) => {
      clearTimeout(timer);
      resolve({ code, out, err });
    });
  });
}

app.get("/api/health", (req, res) => {
  res.json({ ok: true, enginePath: ENGINE_PATH });
});

/**
 * POST /api/generate
 * body: { root: "كتب", scheme: "مفعول" }
 */
app.post("/api/generate", async (req, res) => {
  const { root, scheme } = req.body ?? {};
  if (!root || !scheme) {
    return res.status(400).json({ ok: false, message: "root و scheme مطلوبين" });
  }

  // On appelle le moteur en mode CLI (qu’on ajoute à l’étape 2)
  const args = [
    "--data", ROOTS_PATH,
    "--schemes", SCHEMES_PATH,
    "--generate",
    "--root", root,
    "--scheme", scheme,
    "--json"
  ];

  try {
    const r = await runEngine(args);
    if (r.code !== 0) {
      return res.status(500).json({ ok: false, message: r.err || "خطأ في المحرك" });
    }
    return res.type("json").send(r.out);
  } catch (e) {
    return res.status(500).json({ ok: false, message: e.message });
  }
});

/**
 * POST /api/validate
 * body: { word: "مكتوب", root: "كتب" }
 */
app.post("/api/validate", async (req, res) => {
  const { word, root } = req.body ?? {};
  if (!word || !root) {
    return res.status(400).json({ ok: false, message: "word و root مطلوبين" });
  }

  const args = [
    "--data", ROOTS_PATH,
    "--schemes", SCHEMES_PATH,
    "--validate",
    "--word", word,
    "--root", root,
    "--json"
  ];

  try {
    const r = await runEngine(args);
    if (r.code !== 0) {
      return res.status(500).json({ ok: false, message: r.err || "خطأ في المحرك" });
    }
    return res.type("json").send(r.out);
  } catch (e) {
    return res.status(500).json({ ok: false, message: e.message });
  }
});

const PORT = process.env.PORT || 3001;
app.listen(PORT, "0.0.0.0", () => {
  console.log(`API running on http://localhost:${PORT}`);
});

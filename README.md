# Arabic Morphology Engine

The Arabic language is based on a morphological system called **Root–Pattern**: words are not formed by adding prefixes or suffixes, but by inserting a consonantal root into an abstract morphological pattern.

For example, the root **ك–ت–ب** (to write) combined with the pattern **مفعول** gives **مكتوب** (written), and with **فاعل** gives **كاتب** (writer).

This project implements an engine capable of exploiting this mechanism algorithmically:
- trilateral roots are indexed in an **AVL tree** for O(log n) search,
- morphological patterns are stored in a **hash table** for direct access,
- the engine can **generate** derived words from a root and a pattern, and **validate** whether a word morphologically belongs to a given root.

The whole project is written in **C++17**, with no external library, with native support for **UTF-8/UTF-32** encoding for Arabic characters.

---

## Implemented Features

In accordance with the specifications, the project covers the five main features:

| Feature | Implementation |
|---|---|
| Root management (insertion, search, display) | AVL tree — `src/AVL.cpp` |
| Morphological pattern management | Hash table — `src/hash_table.cpp` |
| Morphological generation (root + pattern → word) | `apply_template()` — `src/morpho.cpp` |
| Morphological validation (word → root + pattern) | `validate_word()` — `src/morpho.cpp` |
| Management of validated derivatives per root | AVL nodes with a list of derivatives and frequencies |

**Additional features:**
- Interactive morphological mini-game in the CLI
- HTTP REST API server (no external dependency)
- Web interface (Next.js) with generation, validation, and game pages

---

## Data Structures

- **AVL tree**: each node stores an Arabic trilateral root, the list of its validated derivatives, and their frequency of occurrence. Comparisons use UTF-32 encoding for correct processing of Arabic characters.
- **Hash table** (open addressing): key = pattern name (e.g. `مفعول`), value = abstract representation of the pattern with its algorithmic transformation rule.

---

## Generation Example

```
Root    : ك–ت–ب
Pattern : مفعول  (template: م–ف–ع–و–ل)
Result  : مكتوب
```

The transformation convention is: `ف` → 1st letter, `ع` → 2nd letter, `ل` → 3rd letter of the root.

---

## Project Structure

```
MoteurMorphologiqueArabe/
├── src/
│   ├── AVL.cpp            AVL tree (root management)
│   ├── hash_table.cpp     Hash table (patterns)
│   ├── morpho.cpp         Generation and validation engine
│   ├── cli_main.cpp       Interactive CLI interface
│   ├── main.cpp           TUI interface (colored menus)
│   ├── server_main.cpp    API server entry point
│   ├── http_server.cpp    Minimal HTTP server (POSIX/Winsock2 sockets)
│   ├── api_routes.cpp     REST routes (generation, validation, game)
│   └── tests.cpp          Unit tests
├── include/               C++ headers
├── data/
│   ├── roots.txt          Arabic trilateral roots
│   ├── schemes.txt        Morphological patterns
│   └── derivatives.json   Validated derivatives per root
├── frontend/              Next.js web interface
├── CMakeLists.txt         CMake configuration
└── Makefile               Make wrapper (Linux / WSL)
```

---

## Compilation

### Prerequisites

- **Native Ubuntu**: `build-essential`, `cmake` >= 3.14
- **WSL (from PowerShell)**: same tools, installed in the Linux environment

### Option 1 — Native Ubuntu

```bash
sudo apt update && sudo apt install -y build-essential cmake
cd MoteurMorphologiqueArabe
make
```

### Option 2 — WSL from PowerShell

```powershell
wsl
```

```bash
sudo apt update && sudo apt install -y build-essential cmake
cd /mnt/c/Users/MSI/MoteurMorphologiqueArabe
make
```

---

## Usage

```bash
cd build
./moteur_cli 
```

---

## Make Commands

| Command | Description |
|---|---|
| `make` | Compiles in Release mode |
| `make debug` | Compiles in Debug mode |
| `make clean` | Removes the `build/` directory |
| `make run-cli` | Launches the CLI |
| `make run-tui` | Launches the interactive TUI |
| `make run-api` | Launches the API server |
| `make run-tests` | Runs the unit tests |
| `make frontend-dev` | Launches the Next.js server |

---

## Web Interface (Frontend)

The project integrates a complete web interface built with **Next.js 14** (React + TypeScript), communicating with the C++ server via HTTP/JSON calls.

### Technical Stack

| Technology | Version | Role |
|-------------|---------|------|
| **Next.js** | 14.2.5 | React framework (App Router) |
| **React** | 18.3 | UI library |
| **TypeScript** | 5.x | Static typing |
| **Tailwind CSS** | 3.4 | Utility styles |
| **Radix UI** | 1.x–2.x | Accessible components (Dialog, Toast, Dropdown…) |
| **Recharts** | 2.13 | Charts (root frequencies) |
| **Lucide React** | 0.460 | Icons |

### Available Pages

| Route | File | Description |
|-------|---------|-------------|
| `/` | `app/page.tsx` | Dashboard |
| `/roots` | `app/roots/page.tsx` | Root management (list, add, delete) |
| `/schemes` | `app/schemes/page.tsx` | Pattern management (list, add, edit, delete) |
| `/generate` | `app/generate/page.tsx` | Generating a word from a root + pattern |
| `/validate` | `app/validate/page.tsx` | Morphological validation of a word |
| `/game` | `app/game/page.tsx` | Interactive morphological mini-game |

### Frontend Architecture

```
frontend/
├── app/                   Next.js pages (App Router)
│   ├── page.tsx           Main dashboard
│   ├── roots/             Root CRUD
│   ├── schemes/           Pattern CRUD
│   ├── generate/          Generate a word
│   ├── validate/          Validate a word
│   ├── game/              Multiple-choice mini-game
│   ├── layout.tsx         Global layout (navbar, RTL)
│   └── globals.css        Global styles (Tailwind)
├── components/
│   ├── layout/            Navbar, Sidebar
│   └── ui/                Reusable components (Button, Card, Toast…)
├── lib/
│   ├── api.ts             C++ API call functions
│   └── types.ts           Shared TypeScript types
├── .env.local             Environment variable (API URL)
└── package.json
```

### Communication with the C++ API

All requests to the backend are centralized in `frontend/lib/api.ts`. The base URL is configured via the environment variable:

```
NEXT_PUBLIC_API_URL=http://localhost:3001
```

**API call examples:**

```typescript
// List all roots
GET  /api/roots

// Add a root
POST /api/roots          { "root": "كتب" }

// Delete a pattern
DELETE /api/schemes/:name

// Generate a word
POST /api/generate        { "root": "كتب", "scheme": "مفعول" }
→ Response: { "success": true, "word": "مكتوب" }

// Validate a word
POST /api/validate        { "word": "مكتوب", "root": "كتب" }
→ Response: { "valid": true, "scheme": "مفعول" }

// Get a game question
GET  /api/game/question
→ Response: { "word": "...", "type": "find_root", "options": [...], "correct_answer": "..." }
```

### Launching the Graphical Interface

#### 1. Launch the C++ API server
```bash
cd build && cmake .. && make
./morpho_api_server 3001
```

#### 2. Launch the Next.js frontend
```bash
cd frontend
npm install
npm run dev
```

The interface is then accessible at **http://localhost:3000**.

> **Note:** The C++ server must be launched first on port **3001** so that the frontend can connect to it (configured in `.env.local`).

## Authors: AYARI Wiem & SAKROUFI Aya

# ============================================================
# Makefile — Moteur Morphologique Arabe
# Wrapper CMake pour Ubuntu / WSL (GNU Make)
# Usage : make [cible]
# ============================================================

BUILD_DIR   := build
BUILD_TYPE  ?= Release
CMAKE_FLAGS ?=

# Détection du nombre de cœurs disponibles
NPROCS := $(shell nproc 2>/dev/null || echo 4)

.PHONY: all configure build debug clean \
        run-cli run-tui run-api run-tests \
        frontend-install frontend-dev help

# ── Cible par défaut ─────────────────────────────────────────
all: build

# ── Configuration CMake ──────────────────────────────────────
configure:
	@echo "[CMake] Configuration en mode $(BUILD_TYPE)..."
	@mkdir -p $(BUILD_DIR)
	cmake -B $(BUILD_DIR) -S . \
	      -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	      $(CMAKE_FLAGS)

# ── Compilation ──────────────────────────────────────────────
build: configure
	@echo "[Make] Compilation ($(NPROCS) cœurs)..."
	cmake --build $(BUILD_DIR) --parallel $(NPROCS)
	@echo "[Make] Binaires disponibles dans $(BUILD_DIR)/"

# ── Compilation Debug ────────────────────────────────────────
debug:
	$(MAKE) build BUILD_TYPE=Debug

# ── Nettoyage ────────────────────────────────────────────────
clean:
	@echo "[Make] Nettoyage du répertoire $(BUILD_DIR)..."
	@rm -rf $(BUILD_DIR)
	@echo "[Make] Nettoyage terminé."

# ── Exécution des cibles ─────────────────────────────────────
run-cli: build
	@echo "[CLI] Lancement du moteur CLI..."
	./$(BUILD_DIR)/moteur_cli

run-tui: build
	@echo "[TUI] Lancement du moteur TUI interactif..."
	./$(BUILD_DIR)/moteur_tui

run-api: build
	@echo "[API] Lancement du serveur API (port 3001)..."
	./$(BUILD_DIR)/morpho_api_server 3001

run-tests: build
	@echo "[Tests] Exécution des tests unitaires..."
	./$(BUILD_DIR)/moteur_tests

# ── Frontend Node.js ─────────────────────────────────────────
frontend-install:
	@echo "[Frontend] Installation des dépendances..."
	cd frontend && npm install

frontend-dev: frontend-install
	@echo "[Frontend] Démarrage du serveur de développement (port 3000)..."
	cd frontend && npm run dev

# ── Aide ─────────────────────────────────────────────────────
help:
	@echo ""
	@echo "  Moteur Morphologique Arabe — Makefile d'aide"
	@echo "  ─────────────────────────────────────────────"
	@echo "  make             → compile en mode Release (défaut)"
	@echo "  make debug       → compile en mode Debug"
	@echo "  make clean       → supprime le répertoire build/"
	@echo "  make run-cli     → lance le CLI"
	@echo "  make run-tui     → lance le TUI interactif"
	@echo "  make run-api     → lance le serveur API HTTP"
	@echo "  make run-tests   → lance les tests unitaires"
	@echo "  make frontend-install → installe les dépendances npm"
	@echo "  make frontend-dev     → lance le frontend Next.js"
	@echo ""

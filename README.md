# Ordonnanceur Linux - Documentation Complète

![License](https://img.shields.io/badge/License-MIT-blue.svg)
![Version](https://img.shields.io/badge/Version-1.0.0-green.svg)
![Status](https://img.shields.io/badge/Status-Production%20Ready-brightgreen.svg)

## 📋 Vue d'ensemble

**Ordonnanceur Linux** est un simulateur complet d'algorithmes d'ordonnancement des processus avec interface web interactive. L'application combine un moteur backend en C pour les simulations d'ordonnancement et une interface web moderne construite avec Next.js/React/TypeScript.

### ✨ Caractéristiques principales

- **6 Algorithmes d'ordonnancement implémentés**:
  - FIFO (First In First Out)
  - Priority (Ordonnancement par Priorité avec Préemption)
  - Round Robin (RR avec Quantum configurable)
  - Multilevel (Multilevel Feedback Queue - Statique)
  - Multilevel Dynamic (Multilevel avec Aging Dynamique)
  - SRT (Shortest Remaining Time)

- **Visualisations avancées**:
  - Diagramme de Gantt dynamique et interactif
  - Graphique d'occupation CPU en temps réel
  - Visualisation de la file d'attente avec animation
  - Statistiques détaillées par processus
  - Graphiques camembert et barres (Recharts)

- **Interface intuitive**:
  - Chargement de fichiers de configuration personnalisés
  - Génération automatique de processus
  - Fichier par défaut (`sample_config.txt`) préchargé
  - Contrôles de lecture (Play/Pause/Step Forward/Step Back)
  - Thème sombre professionnel


### Stack Technologique

#### Backend
- **C (C11)** - Moteur de simulation (GCC)
- **Linux/Unix** - Système d'exploitation cible

#### Frontend
- **Next.js 16.0.3** - Framework React full-stack
- **React 18+** - Bibliothèque UI
- **TypeScript** - Typage statique
- **Tailwind CSS** - Styling utilitaire
- **Recharts** - Bibliothèque graphiques
- **Radix UI** - Composants accessibles
- **Lucide Icons** - Icônes SVG

#### Outils & Infrastructure
- **pnpm** - Gestionnaire de paquets
- **Node.js 18+** - Runtime JavaScript
- **Make** - Build automation (C)
- **Git** - Contrôle de version

### Structure du Projet

```
ordonnanceur-linux/
├── src/                      # Code source C
│   ├── main.c               # Point d'entrée CLI
│   ├── parser.c             # Parser fichiers config
│   ├── scheduler.c          # Orchestrateur simulations
│   └── utils.c              # Utilitaires C
├── policies/                # Algorithmes d'ordonnancement
│   ├── fifo.c
│   ├── priority_preemptive.c
│   ├── roundrobin.c
│   ├── srt.c
│   ├── multilevel.c
│   └── multilevel_dynamic.c
├── include/                 # Headers C
│   ├── process.h           # Structure processus
│   ├── parser.h
│   ├── scheduler.h
│   └── utils.h
├── tests/                   # Tests unitaires C
│   ├── test_fifo.c
│   ├── test_roundrobin.c
│   ├── test_priority.c
│   ├── test_multilevel.c
│   ├── test_multilevel_dynamic.c
│   └── test_parser.c
├── app/                     # Application Next.js
│   ├── page.tsx            # Page principale
│   ├── layout.tsx          # Layout global
│   ├── globals.css         # Styles globaux
│   └── api/
│       ├── parse-config/   # Endpoint parsing
│       └── schedule/       # Endpoint ordonnancement
├── components/              # Composants React
│   ├── results-display.tsx  # Affichage résultats (Gantt, stats)
│   ├── algorithm-selector.tsx
│   ├── file-generation-dialog.tsx
│   ├── theme-provider.tsx
│   └── ui/                 # Composants Radix UI customisés
├── lib/                     # Utilitaires TypeScript
│   ├── types.ts            # Types partagés
│   └── utils.ts
├── config/                  # Fichiers de configuration
│   └── sample_config.txt    # Configuration par défaut
├── public/                  # Assets statiques
├── Makefile                 # Build C
├── package.json             # Dependencies Node.js
├── tsconfig.json            # Configuration TypeScript
├── next.config.mjs          # Configuration Next.js
└── README.md        # Cette documentation
```

---

## 📦 Prérequis

### Windows 10/11
- **WSL2** (Windows Subsystem for Linux) avec Ubuntu 20.04 LTS ou plus récent
- **Git Bash** ou **PowerShell avec WSL intégré**
- **Node.js 18.0.0+** (64-bit)
- **pnpm 8.0.0+**
- **GCC 9.0+** (via apt dans WSL)
- **Make** (via apt dans WSL)

### Linux (Ubuntu/Debian)
- **Node.js 18.0.0+**
- **pnpm 8.0.0+**
- **GCC 9.0+**
- **Make**
- **Build-essential** (compilation outils)

### macOS
- **Xcode Command Line Tools**
- **Node.js 18.0.0+** (via Homebrew)
- **pnpm 8.0.0+**
- **GCC** (via Homebrew: `brew install gcc`)
- **Make** (inclus avec Xcode CLT)

---

## 🚀 Installation & Configuration

### Étape 1: Cloner le repository

```bash
git clone https://github.com/arijsebai/Projet-Ordonnancement-Linux.git
cd Projet-Ordonnancement-Linux
```

### Étape 2: Installer les dépendances Node.js

```bash
# Avec pnpm (recommandé)
pnpm install

# Ou avec npm
npm install
```

### Étape 3: Compiler le backend C

```bash
# Linux/macOS/Windows (WSL)
make clean
make all

# Vérifier la compilation
ls -la ordonnanceur
```

### Étape 4: Vérifier la structure du projet

```bash
# Linux/macOS/Windows (WSL)
ls -la config/sample_config.txt
cat config/sample_config.txt
```

---

## 💻 Utilisation

### Mode 1: Interface Web (Recommandé)

#### Démarrer l'application web

```bash
# Développement (rechargement automatique)
pnpm dev

# L'app sera disponible à : http://localhost:3000
```

#### Interface utilisateur

1. **Page d'accueil** :
   - ✅ Fichier par défaut (`sample_config.txt`) préchargé automatiquement
   - Affichage du nombre de processus chargés
   - Boutons pour "Générer un Fichier" ou "Choisir un Fichier"

2. **Gestion des fichiers** :
   - Générer : Crée 5-10 processus aléatoires
   - Choisir : Uploader un fichier `.txt` personnalisé
   - Format attendu: `name arrival execution priority` (un par ligne), des commenatires et des lignes vides. 

3. **Sélection de l'algorithme** :
   - Choisir parmi FIFO, Priority, RR, Multilevel, Multilevel Dynamic, SRT
   - Paramètres dynamiques (ex: Quantum pour Round Robin)

4. **Lancer la simulation** :
   - Bouton "Lancer l'Ordonnancement"
   - Visualisation en temps réel du Gantt

5. **Résultats** :
   - Diagramme de Gantt avec timeline dynamique
   - Graphique CPU + File d'attente
   - Tableau détaillé des statistiques
   - Graphiques d'analyse (barres, camembert)

#### Exemple de fichier de configuration

```txt
# Format: name arrival_time execution_time priority

P1 0 5 1
P2 2 3 2
P3 4 2 1
P4 6 4 2
P5 8 2 1
```

---

### Mode 2: Ligne de commande (CLI - Backend pur)

#### Exécutable C

```bash
# Format
./ordonnanceur <chemin vers fichier_config.txt>

# Exemple
./ordonnanceur config/sample_config.txt

# Ou bien si vous pouvez générer un nouveau fichier ou enter un fichier de config
./ordonnanceur
```

#### Flux d'exécution CLI

1. Charger le fichier de configuration
2. Menu interactif pour choisir l'algorithme :
   ```
   Choisir un algorithme:
   1. FIFO
   2. Priorité (Préemption)
   3. Round Robin
   4. Multilevel
   5. Multilevel Dynamic
   6. SRT
   ```
3. Entrer les paramètres si nécessaire (ex: Quantum)
4. Simulation et affichage des résultats en console

#### Exemple de sortie console

```
═══════════════════════════════════════════════════════════════
                   DONNÉES DE TEST FIFO
═══════════════════════════════════════════════════════════════
  Name     Arrival   Exec
  ─────    ───────   ────
  P1           0      5
  P2           2      3
  P3           4      2

╔═════════════════════════════════════════════════════════════╗
║            SIMULATION (TABLEAU GANTT)                       ║
╚═════════════════════════════════════════════════════════════╝
 Time  Executing  Ready Queue
 ──────────────────────────────
    0      P1        []
    1      P1        []
    2      P1        [P2]
    3      P1        [P2]
    4      P1        [P2, P3]
    5      P2        [P3]
    6      P2        []
    7      P2        []
    8      P3        []
    9      P3        []

FINAL STATISTICS
Name  Arrival  Exec  Finish  Wait
────────────────────────────────
P1        0     5       5      0
P2        2     3       8      3
P3        4     2      10      4

Average Wait Time: 2.33
Makespan: 10
```

---

## 🧪 Tests

### Tests unitaires C

```bash
# Tester le parser
make clean && make all
./ordonnanceur config/sample_config.txt

# Tester individuellement chaque algorithme
./tests/test_fifo
gcc -Iinclude tests/test_roundrobin.c policies/roundrobin.c -o test_unit && ./test_unit
gcc -Iinclude tests/test_priority.c policies/priority_preemptive.c -o test_unit && ./test_unit
gcc -Iinclude tests/test_multilevel.c policies/multilevel.c -o test_unit && ./test_unit
gcc -Iinclude tests/test_multilevel_dynamic.c policies/multilevel_dynamic.c -o test_unit 
&& ./test_unit
```

### Tests Web

```bash
pnpm build
pnpm start   # serve production build
```

## Configuration file format

```
# Comment (optional)
# NAME ARRIVAL EXECUTION PRIORITY
P1 0 5 1
P2 2 3 2
P3 4 2 1
```

| Field | Type | Description | Example |
|-------|------|-------------|---------|
| NAME | String | Process identifier | P1, Task_A |
| ARRIVAL | Int | Arrival time (≥0) | 0, 5 |
| EXECUTION | Int | Execution time (>0) | 5, 10 |
| PRIORITY | Int | Static priority (small = high) | 0 (high), 5 (low) |

Parsing rules: blank lines ignored; `#` comments ignored (including end-of-line); 4 tokens required per line.

## Scheduling algorithms
- **FIFO** – simplest, non-preemptive; best for batch; convoy effect risk.
- **Priority (preemptive)** – priority-driven; starvation risk; small value high priority (default API).
- **Round Robin** – fair time-slicing; needs quantum (e.g., 2–4); context-switch overhead.
- **Multilevel (static)** – multiple queues + RR; configurable quantum; no priority aging.
- **Multilevel Dynamic (aging)** – multilevel with dynamic priority boosts to prevent starvation; requires quantum.
- **SRT** – shortest remaining time first; great average wait; long jobs may starve.

## Metrics
- Arrival time, Execution time, Finish time
- Waiting time = finish − arrival − execution
- Turnaround time = finish − arrival
- Average waiting time, Makespan

## Troubleshooting

**“Backend binary not found”**
```bash
make clean && make all
ls -la ordonnanceur
```

**“Parse error in config file”**
```bash
cat config/sample_config.txt
# Ensure: NAME ARRIVAL EXECUTION PRIORITY
```

**Port 3000 already in use**
```bash
pnpm dev -- -p 3001
# or
lsof -i :3000
kill -9 <PID>
```

**pnpm not found**
```bash
corepack enable
corepack prepare pnpm@latest --activate
# or: npm install -g pnpm
```

## Provided configs
- `config/sample_config.txt` — default test set
- `config/sample_config_<timestamp>.txt` — generated during web runs

## Contribution
1. Fork the repo
2. Create a branch (`git checkout -b feature/my-change`)
3. Commit (`git commit -m "Add my change"`)
4. Push (`git push origin feature/my-change`)
5. Open a PR

## License
MIT License — see [LICENSE](LICENSE).

## Authors
Academic project (Advanced OS). Team: Arij Sebai, Balkis Hanafi, Hadil Hasni, Aya Sakroufi, Wiem Ayari.

## Support
- Docs: see `Documentation.md`
- Issues: open a GitHub issue with steps, environment, and logs
- Contact: create an issue for questions/suggestions

## Educational use cases
Great for teaching OS scheduling, live demos, student projects, and technical presentations.

## Roadmap (future ideas)
- NUMA-aware scheduling
- CPU/memory profiling
- Export results (PDF, CSV)
- Comparative benchmarking
- Plugin for real-system scheduling
- Mobile UX polish; theme toggle

---
Last updated: December 2025 · Version: 1.0.0
   ```

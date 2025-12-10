# Ordonnanceur Linux - Application Complète

**Simulateur d'ordonnancement multitâche en C** couplé à une **interface web Next.js** pour visualisation interactive des algorithmes de scheduling.

---

## 📋 Table des matières

1. [Présentation générale](#présentation-générale)
2. [Prérequis complets](#prérequis-complets)
3. [Installation et lancement](#installation-et-lancement)
4. [Architecture technique](#architecture-technique)
5. [API Backend C](#api-backend-c)
6. [API Frontend Next.js](#api-frontend-nextjs)
7. [Algorithmes implémentés](#algorithmes-implémentés)
8. [Structures de données](#structures-de-données)
9. [Choix d'architecture](#choix-darchitecture)
10. [Déroulement SCRUM](#déroulement-scrum)
11. [Tests et validation](#tests-et-validation)
12. [Deployment](#deployment)

---

## Présentation générale

Ce projet simule un **ordonnanceur de processus** (scheduler) tel qu'implémenté dans les systèmes d'exploitation Linux/Unix.

### Composants clés

| Composant | Technologie | Rôle |
|-----------|-------------|------|
| **Backend** | C11 + GCC | Simulation des algorithmes, calcul des métriques |
| **Frontend** | React 19 + TypeScript | UI interactive, visualisation |
| **Framework web** | Next.js 16 | Liaison backend/frontend, API routes |
| **Visualisation** | Recharts | Gantt chart, statistiques, camembert |
| **Styles** | Tailwind CSS 4 | Design responsive |
| **UI Components** | Radix UI | Composants accessibles (select, dialog, etc.) |

### Workflow global

```
┌─────────────────────┐
│  UI Next.js/React   │ (Sélection algo, upload config, lancement)
└──────────┬──────────┘
           │ POST /api/schedule
           │ Payload: { processes, algorithm, quantum, ... }
           ▼
┌─────────────────────────────────────────┐
│     API Route: app/api/schedule/route.ts│ (spawn binary C)
├─────────────────────────────────────────┤
│  ordonnanceur --api --config file.txt   │
│             --algo fifo                 │
└──────────┬──────────────────────────────┘
           │ JSON output
           ▼
┌─────────────────────────────────────────┐
│     Backend C (scheduler.c)             │ (Simulation, stats)
│ Gantt: [start, end, process]            │
│ Stats: [id, waitTime, priority, ...]    │
└──────────┬──────────────────────────────┘
           │ Response JSON
           ▼
┌─────────────────────┐
│  Results Display    │ (Charts, table)
└─────────────────────┘
```

---

## Prérequis complets

### Système d'exploitation

- **Linux** : Debian/Ubuntu, Fedora, Arch, etc.
- **macOS** : 11+ (Monterey+)
- **Windows** : MSYS2/MinGW64, WSL2

### Compilateur et outils

| Outil | Version min | Installation |
|-------|-----------|--------------|
| **GCC** ou **Clang** | 11.0 | `sudo apt install gcc` (Linux) / Xcode (macOS) / MSYS2 (Win) |
| **make** | 4.0 | `sudo apt install make` (Linux) / inclus Xcode (macOS) |
| **Node.js** | 20.0 | https://nodejs.org / `nvm install 20` |
| **pnpm** | 9.0 | `npm install -g pnpm` |

### Installation détaillée par OS

#### Linux (Debian/Ubuntu)

```bash
# Dépendances de compilation C
sudo apt update
sudo apt install -y build-essential gcc make

# Node.js + pnpm
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt install -y nodejs
npm install -g pnpm

# Vérification
gcc --version
make --version
node --version
pnpm --version
```

#### macOS

```bash
# Xcode Command Line Tools (inclut GCC, make, etc.)
xcode-select --install

# Node.js + pnpm
brew install node pnpm

# Vérification
gcc --version
node --version
pnpm --version
```

#### Windows (MSYS2/MinGW64)

1. Télécharger et installer MSYS2 : https://www.msys2.org/
2. Lancer MSYS2 MinGW 64-bit terminal
3. Installer les packages

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make

# Node.js + pnpm
pacman -S mingw-w64-x86_64-nodejs
npm install -g pnpm

# Vérification
gcc --version
make --version
node --version
```

### Vérification globale

```bash
gcc --version        # >= 11.0
make --version       # >= 4.0
node --version       # >= 20.0
pnpm --version       # >= 9.0
```

Si l'une des commandes échoue, revérifier l'installation correspondante.

---

## Installation et lancement

### 1. Cloner/télécharger le projet

```bash
cd /chemin/vers/projet
# ou
git clone <url-repo>
cd Projet-Ordonnancement-Linux-arij-dev
```

### 2. Compiler le backend C

```bash
make clean          # Nettoie les anciens fichiers compilés
make                # Compile tout (ou 'make CC=gcc' sous MSYS2)
```

**Résultat** : Un exécutable `ordonnanceur` à la racine (vérifier avec `ls ordonnanceur` ou `dir ordonnanceur.exe` sur Windows).

### 3. Installer les dépendances frontend

```bash
pnpm install        # Installe Node modules
```

### 4. Lancer le serveur de développement

```bash
pnpm dev
```

**Output** :

```
> next dev

  ▲ Next.js 16.0.3
  - Local:        http://localhost:3000
  ...
```

### 5. Accéder l'interface

Ouvrir dans un navigateur : **http://localhost:3000**

### Pour arrêter

- Terminal : `Ctrl+C`
- Compilé : `make clean` (optionnel)

---

## Architecture technique

### Répertoire racine

```
Projet-Ordonnancement-Linux-arij-dev/
├── src/                           # Backend C source
│   ├── main.c                     # CLI, point d'entrée
│   ├── parser.c                   # Lecture fichiers config
│   ├── scheduler.c                # Moteur simulation + JSON API
│   ├── generate_config.c          # Générateur processus
│   └── utils.c                    # Utilitaires affichage
│
├── include/                       # Headers C publics
│   ├── process.h                  # Struct process
│   ├── scheduler.h                # Prototypes scheduler
│   ├── parser.h                   # Prototypes parser
│   └── ...
│
├── policies/                      # Implémentation des algos
│   ├── fifo.c                     # FIFO non-préemptif
│   ├── priority_preemptive.c      # Priority avec préemption
│   ├── roundrobin.c               # Round Robin
│   ├── srt.c                      # SRT
│   ├── multilevel.c               # Multilevel statique
│   └── multilevel_dynamic.c       # Multilevel + aging
│
├── app/                           # Next.js (React frontend)
│   ├── page.tsx                   # Page principale
│   ├── layout.tsx                 # Layout global
│   └── api/
│       ├── schedule/route.ts      # Route: POST /api/schedule
│       └── parse-config/route.ts  # Route: POST /api/parse-config
│
├── components/                    # React components
│   ├── results-display.tsx        # Affichage résultats (Gantt, charts)
│   ├── algorithm-selector.tsx     # Sélecteur algo/params
│   ├── file-generation-dialog.tsx # Dialogue génération fichier
│   └── ui/                        # Composants Radix UI
│
├── lib/                           # Utilitaires TypeScript
│   ├── types.ts                   # Interfaces (Process, Result, etc.)
│   └── utils.ts                   # Helpers
│
├── tests/                         # Tests C unitaires
│   ├── test_fifo.c
│   ├── test_priority.c
│   └── ...
│
├── config/                        # Fichiers config exemple
│   ├── sample_config.txt          # Processus de test
│   └── config_*.txt               # Configs générées dynamiquement
│
├── Makefile                       # Compilation automatisée
├── package.json                   # Dépendances Node.js
├── tsconfig.json                  # Configuration TypeScript
├── tailwind.config.js             # Styles Tailwind
├── next.config.mjs                # Config Next.js
└── README.md                      # Ce fichier
```

### Backend C (architecture interne)

#### Struct `process` (process.h)

```c
struct process {
    char name[64];        // Identifiant unique (P1, P2, ...)
    int arrival_time;     // Temps d'arrivée (unités)
    int exec_time;        // Durée d'exécution (immuable)
    int priority;         // Priorité initiale
    int remaining_time;   // Temps restant à exécuter
    int waiting_time;     // Temps en attente
    int status;           // READY(0), RUNNING(1), ZOMBIE(3)
    int end_time;         // Temps de fin
    int wait_time;        // Pour aging dynamique
};
```

      "end": 80,

```c
struct process_stat {
    char id[64];
    int arrival_time;
    int exec_time;
    int wait_time;
    int finish_time;
    int priority;        // Priorité initiale
    int final_priority;  // Priorité finale (multilevel_dynamic)
};
```

#### Flow d'exécution du backend

1. **main.c** : Parse CLI (`--api`, `--config`, `--algo`, etc.)
2. **parser.c** : Lit le fichier config → tableau de `struct process`
3. **scheduler.c** : Appelle la fonction d'algo (fifo, priority, etc.)
4. **policies/*.c** : Implémente chaque algorithme spécifique
5. **scheduler.c** : Génère JSON → stdout
6. **API Next.js** : Capture stdout → répond en JSON au client

### Frontend (React + Next.js)

#### Flux React

```
Page (app/page.tsx)
├── State: processes[], algorithmConfig, results
├── Actions:
│   ├── handleGenerateFile() → génère processus
│   ├── handleChooseFile() → POST /api/parse-config
│   ├── handleLaunchScheduling() → POST /api/schedule
│   └── handleReset()
├── Renders:
│   ├── FileGenerationDialog
│   ├── AlgorithmSelector
│   ├── ProcessTable (preview)
│   ├── Button "Lancer la simulation"
│   └── ResultsDisplay (si results != null)
└── ResultsDisplay
    ├── Gantt Chart (interactif, timeline)
    ├── Pie Chart (répartition temps total)
    ├── Bar Chart (attente/total par processus)
    └── Table détaillée (id, arrivée, exec, priorité initiale/finale, attente)
```

#### Palette de couleurs déterministe

- Chaque processus reçoit une couleur unique basée sur son ID
- Palette de 20 couleurs + fallback HSL pour garantir l'unicité
- Couleur utilisée partout : Gantt, pie chart, table

---

## API Backend C

### Mode API (JSON)

Le binaire `ordonnanceur` supporte un mode JSON pour intégration système :

```bash
./ordonnanceur --api \
  --config config/sample_config.txt \
  --algo fifo \
  --quantum 2 \
  --prio-order desc
```

### Arguments CLI

| Flag | Valeur | Défaut | Description |
|------|--------|--------|-------------|
| `--api` | (booléen) | - | Active le mode JSON (vs interactif) |
| `--config` | `<fichier>` | - | Chemin fichier config |
| `--algo` | fifo/priority/roundrobin/srt/multilevel/multilevel_dynamic | fifo | Algorithme |
| `--quantum` | `<entier>` | 2 | Quantum pour RR/multilevel |
| `--prio-order` | asc/desc | desc | Ordre priorité (pour priority) |
| `--parse-config` | `<fichier>` | - | Parse et renvoie JSON (au lieu de scheduler) |

### Sortie JSON (mode `--api`)

```json
{
  "algorithm": "fifo",
  "ganttData": [
    {
      "process": "P1",
      "start": 0,
      "end": 5,
      "duration": 5
    },
    {
      "process": "P2",
      "start": 5,
      "end": 80,
      "duration": 3
    }
  ],
  "processStats": [
    {
      "id": "P1",
      "waitTime": 0,
      "totalTime": 5,
      "arrivalTime": 0,
      "executionTime": 5,
      "finishTime": 80,
      "priority": 2
    },
    {
      "id": "P2",
      "waitTime": 3,
      "totalTime": 6,
      "arrivalTime": 2,
      "executionTime": 3,
      "finishTime": 80,
      "priority": 1
    }
  ],
  "averageWait": 1.5,
  "makespan": 80
}
```

### Sortie JSON (mode `--parse-config`)

```json
[
  {
    "id": "P1",
    "arrivalTime": 0,
    "executionTime": 5,
    "priority": 2
  },
  {
    "id": "P2",
    "arrivalTime": 2,
    "executionTime": 3,
    "priority": 1
  }
]
```

---

## API Frontend Next.js

### Route 1: POST /api/parse-config

**Objectif** : Charger un fichier config texte via le backend C.

**Request**

```bash
curl -X POST http://localhost:3000/api/parse-config \
  -F "file=@config/sample_config.txt"
```

**Response**

```json
[
  {
    "id": "P1",
    "arrivalTime": 0,
    "executionTime": 5,
    "priority": 2
  }
]
```

**Code** (app/api/parse-config/route.ts)

```typescript
export async function POST(request: Request) {
  const formData = await request.formData()
  const file = formData.get("file") as File
  
  // Crée un fichier temp
  // Appelle: spawn(ordonnanceur, ['--parse-config', tmpPath])
  // Renvoie le JSON parsé
}
```

### Route 2: POST /api/schedule

**Objectif** : Lancer une simulation avec les paramètres donnés.

**Request**

```bash
curl -X POST http://localhost:3000/api/schedule \
  -H "Content-Type: application/json" \
  -d '{
    "processes": [
      { "id": "P1", "arrivalTime": 0, "executionTime": 5, "priority": 2 },
      { "id": "P2", "arrivalTime": 2, "executionTime": 3, "priority": 1 }
    ],
    "algorithm": "fifo",
    "quantum": 2,
    "priorityOrder": "desc"
  }'
```

**Response**

```json
{
  "algorithm": "fifo",
  "ganttData": [...],
  "processStats": [...],
  "averageWait": 1.5,
  "makespan": 80
}
```

**Code** (app/api/schedule/route.ts)

```typescript
export async function POST(request: Request) {
  const { processes, algorithm, quantum, priorityOrder } = await request.json()
  
  // Crée un fichier temp avec les processus
  // Appelle: spawn(ordonnanceur, ['--api', '--config', tmpPath, '--algo', ...])
  // Parse stdout en JSON
  // Retourne le résultat au client
}
```

---

## Algorithmes implémentés

### 1. FIFO (First-In First-Out)

- **Préemptif** : Non
- **Équité** : Basse (processus long bloque le système)
- **Cas d'usage** : Batch jobs
- **Implémentation** : `policies/fifo.c`

```c
int fifo_scheduler(struct process *procs, int n, int time, int current, int unused) {
    int best = -1;
    int earliest = INT_MAX;
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && 
            procs[i].remaining_time > 0 &&
            procs[i].arrival_time < earliest) {
            earliest = procs[i].arrival_time;
            best = i;
        }
    }
    return best;
}
```

### 2. Priority Preemptive

- **Préemptif** : Oui
- **Modes** : asc (petite valeur = haute prio) / desc (grande valeur = haute prio)
- **Équité** : Basse (processus faible priorité peuvent starver)
- **Implémentation** : `policies/priority_preemptive.c`

```c
int priority_preemptive(struct process *procs, int n, int time, int current, int mode) {
    int best = -1;
    int best_prio = (mode == 0) ? INT_MAX : INT_MIN;
    
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            if ((mode == 0 && procs[i].priority < best_prio) ||
                (mode == 1 && procs[i].priority > best_prio)) {
                best_prio = procs[i].priority;
                best = i;
            }
        }
    }
    return best;
}
```

### 3. Round Robin

- **Préemptif** : Oui (à chaque quantum)
- **Équité** : Haute
- **Quantum** : Configurable
- **Implémentation** : `policies/roundrobin.c`

File circulaire + quantum d'exécution fixe.

### 4. SRT (Shortest Remaining Time)

- **Préemptif** : Oui
- **Temps d'attente** : Théoriquement optimal
- **Complexité** : Haute (calcul du minimum à chaque étape)
- **Implémentation** : `policies/srt.c`

Exécute toujours le processus avec le temps restant le plus court.

### 5. Multilevel Queue (Statique)

- **Préemptif** : Oui
- **Files** : Multiples (par priorité)
- **Équité** : Moyenne
- **Famine** : Oui (basse priorité peut starver)
- **Implémentation** : `policies/multilevel.c`

### 6. Multilevel Feedback Queue (Dynamique) ⭐

- **Préemptif** : Oui
- **Aging** : Priorité augmente au fil du temps en attente
- **Anti-famine** : Oui (vieillissement garantit exécution)
- **Moderne** : Inspiré du CFS Linux réel
- **Implémentation** : `policies/multilevel_dynamic.c` + `scheduler.c` (aging)

**Traçage priorités** :

- `priority` = priorité initiale
- `final_priority` = priorité après vieillissement

---

## Structures de données

### Choix clés

#### 1. Représentation des processus

**Choix** : Tableau statique `struct process[]` alloué dynamiquement

**Avantages** :
- Accès O(1) par indice
- Pas de fragmentation mémoire
- Simplicité implémentation

**Inconvénients** :
- Taille max : 256 processus

```c
struct process *procs = malloc(n * sizeof(struct process));
```

#### 2. Ready Queue

**Choix** : Représentation implicite (parcours du tableau)

```c
for (int i = 0; i < n; i++) {
    if (procs[i].arrival_time <= time && 
        procs[i].remaining_time > 0) {
        // processus prêt
    }
}
```

**Avantages** : Pas de structure auxiliaire → code simple, flexible
**Inconvénients** : O(n) par recherche

#### 3. Gantt Segments

**Structure** :

```c
struct gantt_segment {
    char process[64];
    int start;
    int end;
};
```

Tableau de ~2048 segments max (pour traces détaillées).

#### 4. Process Stats (export JSON)

**Structure** :

```c
struct process_stat {
    char id[64];
    int arrival_time;
    int exec_time;
    int wait_time;
    int finish_time;
    int priority;        // Initiale
    int final_priority;  // Finale (multilevel_dynamic)
};
```

Simplifie conversion → JSON.

---

## Choix d'architecture

### 1. Séparation Backend/Frontend

**Décision** : C (backend) + Next.js (frontend) via API JSON

**Justification** :
- C : performance, bas niveau, requis par projet
- React : UI riche, réactivité, visualisation
- JSON : sérialisation simple, multiplateforme

**Alternative rejetée** : WebAssembly (trop complexe pour un projet étudiant)

### 2. Mode API du binaire C

**Décision** : `--api` flag qui renvoie JSON au lieu d'affichage console

```c
if (api_mode) {
    // Exécute scheduler
    // Appelle print_json_result()
    // stdout = JSON
} else {
    // Mode interactif traditionnel
}
```

**Justification** :
- Une seule compilation du binaire
- Réutilisable en CLI ou via API
- Testable indépendamment

### 3. Fichier temporaire pour config

**Decision** : Node.js crée un fichier temp, appelle `ordonnanceur`, récupère stdout

```typescript
// route.ts
const tmpPath = `/tmp/config_${Date.now()}.txt`;
fs.writeFileSync(tmpPath, configContent);
const result = spawn('ordonnanceur', ['--api', '--config', tmpPath]);
// ... capture stdout ...
fs.unlinkSync(tmpPath); // cleanup
```

**Justification** :
- Format éprouvé (texte)
- Pas de pipe complexe
- Compatible Windows/Linux/macOS

### 4. Couleurs déterministes

**Décision** : Chaque processus (par ID) → couleur fixe via Map

```typescript
const colorMap = new Map<string, string>();
uniqueProcesses.forEach((pid, idx) => {
    const base = PALETTE[idx % 20] || `hsl(${(idx * 137) % 360}deg 70% 45%)`;
    colorMap.set(pid, base);
});
```

**Justification** :
- Palette de 20 couleurs distinctes
- Fallback HSL pour >20 processus (golden angle = 137°)
- Couleur cohérente partout (Gantt, pie, table)

---

## Déroulement SCRUM

### Organisation équipe

- **5 développeurs** : Arij, Aya, Balkis, Hadil, Wiem
- **Durée** : Octobre → Décembre 2025 (10 semaines)
- **Sprints** : 2 semaines chacun (5 sprints)

### Artefacts

#### Product Backlog (initial)

1. Backend C : 6 algorithmes → JSON API
2. Frontend React : UI sélection algo + résultats
3. Intégration C ↔ React
4. Visualisation Gantt
5. Tests unitaires
6. Documentation

#### Sprint Backlog (exemple Sprint 1)

| User Story | Points | Dev | Status |
|-----------|--------|-----|--------|
| Backend C FIFO + Priority | 5 | Arij | Done |
| Frontend: File upload | 3 | Aya | Done |
| Gantt chart (statique) | 5 | Balkis | In Progress |
| API routes setup | 3 | Hadil | Done |

### Réunions

- **Sprint Planning** : Lundi (2h) → définition sprint
- **Daily** : 15 min (Slack/Teams)
- **Review** : Vendredi (1h) → démo résultats
- **Retrospective** : Vendredi (30 min) → améliorations

### Métriques SCRUM

- **Vélocité** : ~15 points/sprint (moyens)
- **Burndown** : Réduction linéaire du backlog
- **Blockers** : Intégration C/JS initiale, puis résolu semaine 2

---

## Tests et validation

### Tests Backend C

Fichiers : `tests/test_*.c`

```bash
# Compiler tests
make test

# Exécuter
./build/test_fifo
./build/test_priority
./build/test_multilevel_dynamic
# ...
```

### Tests Frontend

```bash
# ESLint
pnpm lint

# Build
pnpm build
```

### Tests d'intégration

1. Générer config de test
2. Upload via UI
3. Lancer chaque algo
4. Vérifier Gantt + stats

---

## Deployment

### Développement

```bash
pnpm dev                # Next.js dev server (http://localhost:3000)
./ordonnanceur --api    # Test backend directement
```

### Production

```bash
# Build
make
pnpm build

# Lancer
pnpm start

# Ou containeriser
docker build -t ordonnanceur .
docker run -p 3000:3000 ordonnanceur
```

---

## Format fichier config

Texte, une ligne par processus :

```
NomProcessus TempsArrivée DuréeExécution Priorité
```

Exemple :

```
P1 0 5 2
P2 2 3 1
P3 4 6 3
```

### Validation

- Champ obligatoires : 4 (nom, arrivée, durée, priorité)
- Valeurs numériques : entiers > 0
- Nom : alphanumériques + underscore

---

## Troubleshooting

### `ordonnanceur: command not found`

→ `make` n'a pas compilé. Relancer `make` et vérifier les erreurs.

### Port 3000 déjà utilisé

→ `pnpm dev --port 3001`

### Erreur lors du parse config

→ Vérifier le format du fichier (4 colonnes, séparées par espace).

### API /api/schedule lente

→ Le binaire C peut être lent sur configs massives. Limiter à <100 processus.

---

## Licences et attributions

- Code généré partiellement via assistance IA (ChatGPT, Grok) — voir `ANALYSE_LICENCES.md`
- Radix UI, Recharts, Next.js : licences opensource (MIT, Apache 2.0)

---

## Auteurs

**Équipe ISI 1ING3** (Décembre 2025)

Arij Sebai, Aya Sakroufi, Balkis Hanafi, Hadil Hasni, Wiem Ayari

Institut Supérieur d'Informatique à Ariana, Tunisie

---

## Ressources

- **Documentation C** : https://en.cppreference.com/w/c/
- **Next.js** : https://nextjs.org/docs
- **React** : https://react.dev
- **Recharts** : https://recharts.org

---

## Export en PDF

Ce README (~20-25 pages) peut être exporté en PDF via :

1. **Browser** : Ouvrir ce fichier dans VS Code Preview (Markdown Preview ou GitHub)
   - Imprimer avec `Ctrl+P` → "Enregistrer en PDF"
   - Recommandations : Format A4, Marges min, Pas d'arrière-plan

2. **Ligne de commande** (si pandoc installé) :
   ```bash
   pandoc README.md -o README.pdf --pdf-engine=xelatex
   ```

3. **En ligne** : Copier le contenu dans un convertisseur Markdown→PDF

---

**Dernière mise à jour** : 8 Décembre 2025

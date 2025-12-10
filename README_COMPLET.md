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

---

## 🔧 Architecture Technique

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
└── README_COMPLET.md        # Cette documentation
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
   - Format attendu: `name arrival execution priority` (un par ligne)

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
./ordonnanceur <fichier_config.txt>

# Exemple
./ordonnanceur config/sample_config.txt
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
./tests/test_roundrobin
./tests/test_priority
./tests/test_multilevel
./tests/test_multilevel_dynamic
```

### Tests Web

```bash
# Build de production
pnpm build

# Run en mode production
pnpm start

# Lint & vérifications
pnpm lint
```

---

## 📊 Format de Configuration

### Structure du fichier

```
# Commentaire (optionnel)
# Format: NAME ARRIVAL EXECUTION PRIORITY

P1 0 5 1
P2 2 3 2
P3 4 2 1
```

### Champs obligatoires

| Champ | Type | Description | Exemple |
|-------|------|-------------|---------|
| NAME | String | Identifiant du processus | P1, P2, Task_A |
| ARRIVAL | Entier | Temps d'arrivée (≥0) | 0, 5, 10 |
| EXECUTION | Entier | Temps d'exécution (>0) | 5, 10, 3 |
| PRIORITY | Entier | Priorité statique (≥0) | 0 (haute) à 5 (basse) |

### Règles de parsing

- Les lignes vides sont ignorées
- Les commentaires `#` sont ignorés
- Les commentaires en fin de ligne sont acceptés
- Minimum 4 tokens par ligne
- Les priorités peuvent être inversées selon l'algorithme

---

## 🎯 Algorithmes d'ordonnancement

### 1. FIFO (First In First Out)
- **Description**: Plus simple, non-préemptif
- **Cas d'usage**: Système batch, démonstration pédagogique
- **Avantages**: Simple, déterministe
- **Inconvénients**: Peut avoir long temps d'attente (convoy effect)

### 2. Priority (Ordonnancement par Priorité)
- **Description**: Préemptif basé sur la priorité
- **Cas d'usage**: Systèmes temps réel, multi-tâche
- **Paramètres**: Priorité initiale
- **Avantages**: Processus critique d'abord
- **Inconvénients**: Risque de famine (starvation)

### 3. Round Robin
- **Description**: Partage équitable du CPU (quantum)
- **Cas d'usage**: Systèmes interactifs, multi-utilisateurs
- **Paramètres**: Quantum (ex: 2-4 unités temps)
- **Avantages**: Équitable, réactivité
- **Inconvénients**: Context switch overhead

### 4. Multilevel (Feedback Queue Statique)
- **Description**: Files d'attente par priorité avec Round Robin
- **Cas d'usage**: Systèmes batch + interactif
- **Paramètres**: Quantum
- **Avantages**: Flexibilité, équité
- **Inconvénients**: Configuration complexe

### 5. Multilevel Dynamic (avec Aging)
- **Description**: Multilevel avec ajustement dynamique de priorité
- **Cas d'usage**: Prévention de famine, équité améliorée
- **Paramètres**: Quantum, taux aging
- **Avantages**: Évite la famine, équitable
- **Inconvénients**: Overhead computationnel

### 6. SRT (Shortest Remaining Time)
- **Description**: Préemptif, privilégie les jobs courts
- **Cas d'usage**: Minimiser temps moyen attente
- **Avantages**: Bon temps d'attente moyen
- **Inconvénients**: Starvation des longs jobs

---

## 📈 Métriques et Statistiques

L'application calcule et affiche:

- **Temps d'arrivée** : Quand le processus arrive
- **Temps d'exécution** : Durée totale requise
- **Temps de fin** : Moment de completion
- **Temps d'attente** : Temps passé en file (finish - arrival - execution)
- **Temps de rotation** : Temps total du processus (finish - arrival)
- **Makespan** : Temps total de la simulation
- **Temps d'attente moyen** : Moyenne pour tous les processus

---

## 🐛 Troubleshooting

### Problème: "Binaire backend introuvable"
```bash
# Solution
make clean
make all
# Vérifier que ordonnanceur existe
ls -la ordonnanceur
```

### Problème: "Erreur lors du parsing du fichier"
```bash
# Vérifier le format du fichier
cat config/sample_config.txt

# Valider chaque ligne
# Format: NAME ARRIVAL EXECUTION PRIORITY
P1 0 5 1
P2 2 3 2
```

### Problème: "Port 3000 déjà utilisé"
```bash
# Spécifier un autre port
pnpm dev -- -p 3001

# Ou tuer le processus existant
lsof -i :3000
kill -9 <PID>
```

### Problème: "pnpm: commande introuvable" (Windows WSL)
```bash
# Installer pnpm globalement
npm install -g pnpm

# Ou utiliser npm directement
npm install
npm run dev
```

---

## 🔐 Sécurité & Performance

### Limitations connues
- Simulation limitée à ~100 processus
- Quantum minimum recommandé: 1
- Nom processus: max 64 caractères
- Temps exécution: max 10000 unités

### Optimisations
- Parsing streaming pour gros fichiers
- Memoization des calculs UI
- Virtual scrolling pour longs tableaux
- Debouncing des contrôles timeline

---

## 📝 Fichiers de Configuration Fournis

### `config/sample_config.txt`
Fichier par défaut avec 7 processus de test

### `config/sample_config_<timestamp>.txt`
Fichiers générés lors de chaque simulation web

---

## 🤝 Contribution

Les contributions sont les bienvenues! 

1. Fork le repo
2. Créer une branche (`git checkout -b feature/AmazingFeature`)
3. Commit vos changements (`git commit -m 'Add AmazingFeature'`)
4. Push vers la branche (`git push origin feature/AmazingFeature`)
5. Ouvrir une Pull Request

---

## 📄 Licence

Ce projet est licencié sous la Licence MIT - voir le fichier [LICENSE](LICENSE) pour les détails.

```
MIT License

Copyright (c) 2025 arijsebai

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software...
```

---

## 👨‍💻 Auteur

**Arij Sebai**
- GitHub: [@arijsebai](https://github.com/arijsebai)
- Email: contact@arijsebai.dev
- Projet: [Projet-Ordonnancement-Linux](https://github.com/arijsebai/Projet-Ordonnancement-Linux)

---

## 📞 Support

### Documentation
- Voir `Documentation.md` pour détails techniques
- Voir `INDEX.md` pour index des fichiers
- Voir `COMPLETION_SUMMARY.md` pour historique

### Issues
Signaler des bugs via GitHub Issues avec:
- Description claire du problème
- Étapes de reproduction
- Environnement (OS, versions)
- Logs pertinents

### Contact
Pour questions ou suggestions, créer une issue sur le repository.

---

## 🎓 Cas d'usage pédagogiques

Cette application est idéale pour:
- Cours sur les systèmes d'exploitation
- Visualisation d'algorithmes d'ordonnancement
- Démonstrations en temps réel
- Projets étudiants
- Présentations techniques

---

## ⚡ Roadmap Futures

- [ ] Support NUMA/NUMA-aware scheduling
- [ ] Profiling CPU/Mémoire
- [ ] Export résultats (PDF, CSV)
- [ ] Benchmarking comparatif
- [ ] Plugin système pour scheduling réel
- [ ] Mobile responsive improvements
- [ ] Dark/Light theme toggle

---

## 🙏 Remerciements

- Radix UI pour composants accessibles
- Recharts pour visualisations
- Vercel pour Next.js
- Communauté Linux et système d'exploitation

---

**Dernière mise à jour**: Décembre 2025  
**Version**: 1.0.0 - Production Ready

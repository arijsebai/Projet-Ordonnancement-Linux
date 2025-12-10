# 📚 Documentation Complète - Ordonnanceur Linux

Cette documentation a été **mise à jour en décembre 2025** avec une analyse complète du projet.

---

## 📖 Sélectionner votre documentation

### 🚀 Pour un démarrage RAPIDE (5 min)

👉 **Voir la section "Démarrage rapide" du [README.md](README.md#-démarrage-rapide)**

Contient:
- Installation en 4 étapes
- Lancement mode web
- Lancement mode CLI

---

### 📋 Pour documentation EXHAUSTIVE et COMPLÈTE

👉 **Consulter: [`README_COMPLET.md`](README_COMPLET.md)** (50+ pages)

**Contenu**:
- ✅ Prérequis détaillés (Windows/Linux/macOS)
- ✅ Installation étape par étape
- ✅ Architecture technique complète
- ✅ Stack technologique détaillée
- ✅ 6 Algorithmes expliqués
- ✅ Format de fichier configuration
- ✅ Utilisation web complète
- ✅ Utilisation CLI complète
- ✅ Exemples de sortie console
- ✅ Tests et validation
- ✅ Troubleshooting détaillé
- ✅ Métriques et statistiques
- ✅ Sécurité et performance
- ✅ Roadmap future

---

### 🔧 Pour les TECHNICIENS et CONTRIBUTEURS

👉 **Voir: [`Documentation.md`](Documentation.md)**

Contient:
- Architecture interne C
- Structures de données
- Choix d'architecture
- Workflow SCRUM
- Détails API
- Tests unitaires

---

### 📑 Pour l'INDEX et organisation

👉 **Voir: [`INDEX.md`](INDEX.md)**

Index complet de tous les fichiers et leur description.

---

### 📊 Pour HISTORIQUE des modifications

👉 **Voir: [`COMPLETION_SUMMARY.md`](COMPLETION_SUMMARY.md)** et [`UPDATE_SUMMARY.md`](UPDATE_SUMMARY.md)**

Historique détaillé des changements et améliorations.

---

## 🎯 Guide de sélection rapide

| Besoin | Lire | Temps |
|--------|------|-------|
| **Installer et utiliser rapidement** | README.md + sections rapides | 5 min ⚡ |
| **Comprendre tout le projet** | README_COMPLET.md | 20 min 📖 |
| **Développer/Contribuer** | Documentation.md | 30 min 🔧 |
| **Retrouver un fichier** | INDEX.md | 2 min 🔍 |
| **Voir les changements** | UPDATE_SUMMARY.md | 5 min 📝 |

---

## 📦 Prérequis essentiels (Résumé)

### Windows
- WSL2 + Ubuntu 20.04+
- Node.js 18+, GCC 9+, Make

### Linux (Ubuntu/Debian)
```bash
sudo apt install build-essential gcc make nodejs
npm install -g pnpm
```

### macOS
```bash
xcode-select --install
brew install node gcc pnpm
```

---

## 🚀 Installation standard (4 étapes)

```bash
# 1. Clone
git clone https://github.com/arijsebai/Projet-Ordonnancement-Linux.git
cd Projet-Ordonnancement-Linux

# 2. Dépendances
pnpm install

# 3. Compile backend C
make clean && make all

# 4. Démarre app
pnpm dev
# → http://localhost:3000
```

---

## 🌐 Mode web

```bash
pnpm dev              # Développement
# ou
pnpm build && pnpm start  # Production
```

**Interface** :
1. Fichier par défaut préchargé
2. Générer/Charger fichier
3. Sélectionner algorithme
4. Lancer simulation
5. Visualiser: Gantt + Stats

---

## 💻 Mode CLI (backend C)

```bash
./ordonnanceur config/sample_config.txt

# Menu interactif:
# 1. FIFO
# 2. Priority
# 3. Round Robin
# 4. Multilevel
# 5. Multilevel Dynamic
# 6. SRT
```

---

## 📚 Table des matières complète

### README principal
- Démarrage rapide
- Prérequis
- Installation
- Utilisation web
- Utilisation CLI
- Algorithmes
- Stack tech
- Support

### README_COMPLET.md (⭐ Documentation exhaustive)
- Vue d'ensemble complète
- Prérequis détaillés par OS (Windows/Linux/macOS)
- Installation avec étapes
- Architecture technique (Frontend/Backend)
- 6 Algorithmes expliqués en détail
- Format fichier configuration
- Métriques et statistiques
- Tests et validation
- Troubleshooting complet
- Sécurité et performance
- Licences et attributions
- Roadmap futures

### Documentation.md
- Architecture technique interne
- Structures de données
- Choix d'architecture
- Workflow SCRUM détaillé
- Détails API (backend C + Next.js)
- Tests unitaires

### INDEX.md
- Index complet des fichiers
- Description par répertoire
- Points d'entrée par use-case

### COMPLETION_SUMMARY.md
- Résumé de complétion du projet
- Features implémentées
- État des algorithmes

### UPDATE_SUMMARY.md
- Historique détaillé des updates
- Changements récents
- Améliations

---

## 🎓 Cas d'usage

Parfait pour:
- 📚 Cours systèmes d'exploitation
- 🎯 Démonstrations d'algorithmes
- 🧪 Projet universitaire
- 📊 Présentations techniques
- 🔬 Recherche scheduling

---

## 🤝 Contribution

1. Fork le repo
2. Créer branche feature
3. Commit + push
4. Pull request

Pour détails: voir **README_COMPLET.md → Contribution**

---

## 📞 Support

- 🐛 **Issues** → GitHub Issues avec détails
- 📖 **Docs** → Consulter documentation présente
- 💬 **Questions** → Créer une issue "Question"

---

## ✨ Highlights

- ✅ **6 algorithmes implémentés** (FIFO, Priority, RR, SRT, Multilevel x2)
- ✅ **Interface web moderne** (Next.js + React + Recharts)
- ✅ **Backend performant en C** (GCC)
- ✅ **Visualisation Gantt interactive**
- ✅ **Statistiques détaillées**
- ✅ **Documentation complète** (50+ pages)
- ✅ **Mode web + CLI**
- ✅ **Tests unitaires**
- ✅ **Responsive design**
- ✅ **Production ready**

---

## 🎯 Conclusion

Cette documentation offre:
1. **Accès rapide** au démarrage (5 min)
2. **Compréhension complète** du projet (30-60 min)
3. **Guide de contribution** pour développeurs
4. **Troubleshooting** pour utilisateurs

**Démarrer maintenant**: [`README.md`](README.md#-démarrage-rapide) → 5 minutes ⚡

---

**Dernière mise à jour**: Décembre 2025  
**Version**: 1.0.0 - Production Ready  
**License**: MIT  
**Auteur**: Arij Sebai & Équipe

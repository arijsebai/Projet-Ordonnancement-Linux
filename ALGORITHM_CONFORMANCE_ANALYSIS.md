# 📋 Analyse de Conformité - Sections Algorithmes (3.1 à 3.6)

**Date**: 13 Décembre 2025  
**Fichier analysé**: `Documentation.md` sections 3.1 à 3.6  
**Code source vérifié**: `policies/*.c` et `src/scheduler.c`

---

## 📊 Résumé Exécutif

| Algorithme | Section | Statut | Problèmes |
|------------|---------|--------|-----------|
| **FIFO** | 3.1 | ✅ **CONFORME** | Aucun |
| **Priority Preemptive** | 3.2 | ✅ **CONFORME** | Aucun |
| **Round Robin** | 3.3 | ⚠️ **PARTIELLEMENT** | 3 problèmes identifiés |
| **SRT** | 3.4 | ✅ **CONFORME** | Aucun |
| **Multilevel Static** | 3.5 | ❌ **INCOHÉRENT** | Convention priorité inversée |
| **Multilevel Dynamic** | 3.6 | ✅ **CONFORME** | Aucun (aging implémenté) |

**Conformité globale**: **67%** (4 sections sur 6 sans problèmes)

---

## ✅ **Section 3.1 - FIFO (First-In First-Out)**

### Verdict: **ENTIÈREMENT CONFORME**

#### Implémentation vérifiée

**Fichier**: `policies/fifo.c` (19 lignes de logique)

```c
int fifo_scheduler(struct process *procs, int n, int time, int current, int prio_mode) {
    int best_index = -1;
    int earliest_arrival = INT_MAX;

    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            if (procs[i].arrival_time < earliest_arrival) {  // ← clef
                earliest_arrival = procs[i].arrival_time;
                best_index = i;
            }
        }
    }
    return best_index;
}
```

#### Conformité par élément

| Doc Element | Code | Correspondance | ✓ |
|-------------|------|-----------------|---|
| Sélection processus "prêt" | `arrival_time <= time && remaining_time > 0` | Exact match ✓ | ✅ |
| Critère: "arrival_time le plus bas" | `if (procs[i].arrival_time < earliest_arrival)` | Implémenté correctement | ✅ |
| Retour -1 si vide | `return best_index; // -1 si aucun` | Déclaration initiale à -1 | ✅ |
| Non-préemptif | `(void)current;` non utilisé | Aucune check sur processus courant | ✅ |
| Inactivité CPU | `scheduler.c:175` gère `idx == -1 → [IDLE]` | Affichage implémenté | ✅ |

#### Cas d'usage documenté

| Cas | Documentation | Réalité |
|-----|---|---|
| Batch jobs | "Linux/Unix batch jobs" | ✅ Cas réel documenté |
| Convoy effect | "Un processus long bloque tout" | ✅ Cas réel documenté |
| Avantages/Inconvénients | Table 6 points | ✅ Tous valides pour le code |

---

## ✅ **Section 3.2 - Priority Preemptive**

### Verdict: **ENTIÈREMENT CONFORME**

#### Implémentation vérifiée

**Fichier**: `policies/priority_preemptive.c` (27 lignes)

```c
int priority_preemptive(struct process *procs, int n, int time, int current, int prio_mode) {
    global_prio_mode = prio_mode;
    int best = -1;
    int best_prio = prio_mode ? -99999 : 99999;  // ← modes inversés correctement

    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            int pr = procs[i].priority;
            if ((prio_mode && pr > best_prio) || (!prio_mode && pr < best_prio)) {
                best_prio = pr;
                best = i;
            }
        }
    }
    return best;
}
```

#### Conformité par élément

| Doc Element | Code | Correspondance | ✓ |
|-------------|------|-----------------|---|
| **Mode Descending (1)** | `prio_mode ? -99999 : ...` init | Cherche `pr > best_prio` | ✅ Grande val = haute prio |
| **Mode Ascending (0)** | `!prio_mode && pr < best_prio` | Cherche `pr < best_prio` | ✅ Petite val = haute prio |
| **Convention opposée** | Doc dit "Mode Ascending: petite val" | Code: `best_prio = 99999; ... pr < best_prio` | ✅ Exact |
| **Préemption** | `(void)current;` ignoré | Pas de dépendance au processus courant | ✅ Préemptif garanti |
| **CLI défaut** | Doc: "main.c: prio_mode=1 (descending)" | `main.c:28`: `int prio_mode = 1;` | ✅ Exact |
| **API mapping** | Doc: "route passe --prio-order asc → 0" | `main.c:56`: `prio_mode = strcmp(val, "asc") == 0 ? 0 : 1` | ✅ Exact |

#### Avantages/Inconvénients

| Point | Documentation | Validité |
|-------|---|---|
| "Processus critiques prioritaires" | ✅ Vrai pour mode décroissant |
| "Risque famine pour basse priorité" | ✅ Vrai, aucun mécanisme anti-famine |
| "Systèmes temps réel dur" | ✅ Cas réel valide |
| "Overhead context switches" | ✅ Vrai en préemptif |

---

## ⚠️ **Section 3.3 - Round Robin**

### Verdict: **PARTIELLEMENT CONFORME** - 3 Problèmes identifiés

#### Implémentation vérifiée

**Fichier**: `policies/roundrobin.c` (104 lignes)

### ❌ **Problème 1: Détection de duplication manquante**

**Doc (Étape 2.1, ligne 368)**:
> "Processus pas déjà présent dans la queue (vérification explicite)"
> "Pour chaque candidat, parcourir la queue actuelle [head, tail) - Vérifier si l'indice du processus est déjà dans ready[j]"

**Code réel (lignes 28-33)**:
```c
for (int i = 0; i < n; i++) {
    if (p[i].arrival_time == time && p[i].remaining_time > 0) {
        ready[tail++] = i;  // ← Pas de vérification de duplication!
    }
}
```

**Impact**: 
- Les processus peuvent être ajoutés plusieurs fois à la queue
- Cause une exécution incorrecte si le processus est dans `ready[]` plusieurs fois
- **Severité**: 🔴 HIGH - Cause résultats incorrects

**Correction suggérée**: Ajouter boucle de vérification:
```c
int already_in_queue = 0;
for (int j = head; j < tail; j++) {
    if (ready[j] == i) { already_in_queue = 1; break; }
}
if (!already_in_queue) ready[tail++] = i;
```

---

### ⚠️ **Problème 2: Waiting time non accumulé correctement**

**Doc (Étape 3.4, ligne 373)**:
> "Pour tous les processus **encore en queue** (de head à tail):
> - `waiting_time += run` (ils attendent pendant que curr s'exécute)"

**Code réel**: 
- Aucune ligne n'accumule `waiting_time` pendant l'exécution
- La variable `waiting_time` est initialisée à 0 mais **jamais modifiée**

```c
for (int i = 0; i < n; i++) {
    p[i].waiting_time = 0;  // Initialisé une fois
    // ... jamais mis à jour après!
}
```

**Impact**:
- Les statistiques finales affichent temps d'attente = 0 pour tous les processus
- **Severité**: 🔴 HIGH - Résultats statistiques invalides

**Correction suggérée**: Ajouter après exécution:
```c
for (int j = head; j < tail; j++) {
    p[ready[j]].waiting_time += run;
}
```

---

### ❌ **Problème 3: Réémission implicite vs documentation**

**Doc (Étape 5.1, ligne 378)**:
> "Si le processus n'est pas terminé (remaining_time > 0):
> - Le remettre en fin de queue: ready[tail++] = curr"

**Code réel (lignes 36-48)**:
```c
if (current_idx == -1 || quantum_counter >= quantum || p[current_idx].remaining_time == 0) {
    if (current_idx != -1 && p[current_idx].remaining_time > 0) {
        ready[tail++] = current_idx;  // Réémission
    }
    // ... sélectionner next
}
```

**Divergence**: 
- Doc décrit une "réémission à chaque quantum expiré"
- Code la place dans le bloc d'initialisation du processus suivant
- **Impact**: Logic is functionally equivalent but organization unclear

**Severité**: 🟡 MEDIUM - Confusion sur l'ordre des opérations

---

### Tableau Récapitulatif - Round Robin

| Étape Doc | Code Correspondant | Statut | Conformité |
|-----------|-------------------|--------|-----------|
| 1. Initialisation | `roundrobin.c:7-15` | ✅ Correct | ✅ |
| 2.1 Ajout nouveaux arrivants | `roundrobin.c:28-33` | ❌ Pas de vérification | ❌ MANQUANT |
| 2.2 Vérifier queue vide | `roundrobin.c:36-49` | ⚠️ Simplifié | ⚠️ |
| 3.1-3.5 Sélection et exécution | `roundrobin.c:50-70` | ✅ Correct | ✅ |
| 3.4 Waiting time update | ❌ PAS D'IMPLÉMENTATION | ❌ Absent | ❌ MANQUANT |
| 5.1 Réémission en queue | `roundrobin.c:39` | ✅ Présent | ✅ |
| 7. Statistiques finales | `scheduler.c` | ⚠️ waiting_time = 0 | ❌ INCORRECTE |

---

## ✅ **Section 3.4 - SRT (Shortest Remaining Time First)**

### Verdict: **ENTIÈREMENT CONFORME**

#### Implémentation vérifiée

**Fichier**: `policies/srt.c` (95 lignes)

#### Logique de sélection

```c
int best = -1;
int min_rem = 999999;

for (int i = 0; i < n; i++) {
    if (p[i].arrival_time <= time && p[i].remaining_time > 0) {
        if (p[i].remaining_time < min_rem ||
           (p[i].remaining_time == min_rem && p[i].arrival_time < p[best].arrival_time)) {
            min_rem = p[i].remaining_time;
            best = i;
        }
    }
}
```

#### Conformité par élément

| Doc Element | Code | Correspondance | ✓ |
|-------------|------|-----------------|---|
| Sélection temps restant min | `remaining_time < min_rem` | Exact match ✓ | ✅ |
| FIFO tie-break | `remaining_time == min_rem && arrival_time <` | Implémenté correctement | ✅ |
| Préemption | À chaque `time++`, nouvelle sélection | Garantie | ✅ |
| Statut IDLE | `if (best == -1) → [IDLE]` | Correct | ✅ |
| Statistiques | `wait_time = end_time - arrival_time - exec_time` | Standard académique ✅ | ✅ |
| Famine possible | "Processus long jamais sélectionné" | ✅ Cas réel documenté | ✅ |

#### Cas d'usage

| Cas | Documentation | Réalité |
|-----|---|---|
| "Aucun en production" | ✅ Vrai, requiert avenir |
| "Théorique uniquement" | ✅ Utilisation académique |
| "Temps attente très bon" | ✅ Vrai mathématiquement |

---

## ❌ **Section 3.5 - Multilevel Queue (Statique)**

### Verdict: **INCOHÉRENT** - Convention priorité inversée

#### Problème Principal: Convention de Priorité

**Doc (ligne 568, Principe)**:
> "Convention de priorité: Petite valeur = Haute Priorité (ex: 1 > 10, conforme Unix)"

**Code réel (`multilevel.c` lignes 11-16)**:
```c
int best_prio = -1;  // Cherche le MAXIMUM
for (int i = 0; i < n; i++) {
    if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
        if (procs[i].priority > best_prio) {  // ← Cherche GRANDE valeur!
            best_prio = procs[i].priority;
        }
        processes_ready = 1;
    }
}
```

**Contradiction directe**:
| Aspect | Doc | Code | Conformité |
|--------|-----|------|-----------|
| Convention déclarée | "Petite = haute (Unix)" | "Grande = haute" | ❌ OPPOSÉE |
| Implémentation | Petite priorité sélectionnée | Grande priorité sélectionnée | ❌ OPPOSÉE |
| Exemple | "1 > 10" (1 meilleur) | Sélectionne 10 plutôt que 1 | ❌ INVERSE |

#### Impact

**Fonctionnalité**:
- L'algorithme **fonctionne**, mais avec convention **inversée**
- Un processus avec priorité `priority=10` s'exécute AVANT `priority=1`
- C'est l'opposé de ce qui est documenté

**Severité**: 🔴 HIGH - Incohérence sémantique majeure

#### Autres éléments

| Étape | Implémentation | Statut |
|-------|---|---|
| Trouver priorité min parmi prêts | Inverted: finds max instead | ❌ INVERSÉ |
| Continuer processus courant | `!quantum_expired && priority == best_prio` | ✅ Correct |
| Round-Robin circulaire | `start_index = (current + 1) % n` | ✅ Correct |
| Retour IDLE | `if (!processes_ready) return -1` | ✅ Correct |

#### Correction nécessaire

**Option 1**: Changer le code pour respecter la convention Unix
```c
int best_prio = INT_MAX;  // Cherche le MINIMUM
if (procs[i].priority < best_prio) {  // Petite valeur
    best_prio = procs[i].priority;
}
```

**Option 2**: Changer la doc pour refléter la convention du code
- Remplacer "Petite valeur = haute prio (Unix)" par "Grande valeur = haute prio"
- Remplacer exemple "1 > 10" par "10 > 1"

---

## ✅ **Section 3.6 - Multilevel Feedback Queue (Dynamique)**

### Verdict: **ENTIÈREMENT CONFORME** (avec clarifications)

#### Implementation vérifiée

**Fichier 1**: `policies/multilevel_dynamic.c` (sélection, 37 lignes)  
**Fichier 2**: `src/scheduler.c` lignes 384-470 (boucle simulation)

#### Partie 1: Sélection (multilevel_dynamic.c)

```c
int select_multilevel_dynamic(struct process *procs, int n, int time, int current, int quantum_expired) {
    int best_prio = -1;
    int processes_ready = 0;
    
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            if (procs[i].priority > best_prio) {
                best_prio = procs[i].priority;
            }
            processes_ready = 1;
        }
    }
    if (!processes_ready) return -1;
    
    // Round-Robin logic...
}
```

**Conformité**: ✅ Identique à Multilevel Static (même convention)

#### Partie 2: Aging Loop (scheduler.c, lignes 416-424)

**Doc (étape 2.3)**:
> "Pour tous les processus en attente:
> - `priority++` (augmentation de priorité à chaque cycle)
> - `waiting_time++` (compteur d'attente)
> - Garantie: Aucun processus ne peut attendre indéfiniment"

**Code réel**:
```c
/* Aging dynamique */
for (int i = 0; i < n; i++) {
    if (i != idx && procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
        procs[i].priority++;       // ← montée dynamique
        procs[i].waiting_time++;   // ← compteur d'attente
    }
}
```

**Conformité**: ✅ **EXACT MATCH**

#### Partie 3: Statistiques finales (scheduler.c, lignes 450-470)

**Doc**: "Afficher pour chaque processus: Name, Arrival, Exec, Finish, Wait, Final_Prio"

**Code** (ligne 462):
```c
printf("%-4s  %7d  %4d  %6d  %4d  %10d\n",
       procs[i].name, procs[i].arrival_time, procs[i].exec_time,
       procs[i].end_time, wait, procs[i].priority);  // ← priorité finale
```

**Conformité**: ✅ **PRÉSENT**

#### Tableau Récapitulatif - Multilevel Dynamic

| Étape | Code | Statut | Notes |
|-------|------|--------|-------|
| Sélection (étape 1-3) | `multilevel_dynamic.c` | ✅ | Identique à static |
| Aging (étape 2.3) | `scheduler.c:416-424` | ✅ | Implémenté exactement |
| Garantie anti-famine | `priority++` en boucle | ✅ | Garantie mathématique |
| Quantum handling | `quantum_counter >= quantum` | ✅ | Correct |
| Statistiques | `scheduler.c:462` | ✅ | Final_Prio affiché |
| Affichage état | `scheduler.c:406-413` | ✅ | Ready queue affichée |

#### Cas particulier: Convention priorité

**Note**: Multilevel Dynamic utilise AUSSI la convention "grande valeur = haute prio" (même que Static).

Cela signifie:
- L'aging augmente les priorités avec `priority++`
- Un processus avec priorité initiale 5 deviendra 6, 7, 8, ... au fil du temps
- Après assez de cycles, tous les processus atteindront la même priorité
- À ce stade, il y a équité de sélection
- **Famine éliminée** ✅

---

## 📋 Résumé par Section

### Tableau Global

```
┌─────────────────────────────────────────────────────────────────┐
│                    CONFORMANCE MATRIX                           │
├────────────┬──────────┬────────────────┬────────────────────────┤
│ Section    │ Algo     │ Statut         │ Problèmes              │
├────────────┼──────────┼────────────────┼────────────────────────┤
│ 3.1        │ FIFO     │ ✅ CONFORME    │ Aucun                  │
│ 3.2        │ Priority │ ✅ CONFORME    │ Aucun                  │
│ 3.3        │ RR       │ ⚠️  PARTIEL    │ 3 problèmes            │
│ 3.4        │ SRT      │ ✅ CONFORME    │ Aucun                  │
│ 3.5        │ MLQ      │ ❌ INCOHÉRENT  │ Convention inversée     │
│ 3.6        │ MLFDQ    │ ✅ CONFORME    │ Aucun                  │
└────────────┴──────────┴────────────────┴────────────────────────┘
```

### Détails par Section

| Section | Conforme | Erreurs | Warnings | Severité |
|---------|----------|---------|----------|----------|
| 3.1 FIFO | ✅ 100% | 0 | 0 | - |
| 3.2 Priority | ✅ 100% | 0 | 0 | - |
| 3.3 Round Robin | ⚠️ 50% | 2 | 1 | 🔴 HIGH |
| 3.4 SRT | ✅ 100% | 0 | 0 | - |
| 3.5 Multilevel Static | ⚠️ 50% | 1 | 0 | 🔴 HIGH |
| 3.6 Multilevel Dynamic | ✅ 100% | 0 | 0 | - |
| **GLOBAL** | **67%** | **3** | **1** | **MEDIUM** |

---

## 🔧 Recommandations de Correction

### Priorité 1: 🔴 HAUTE - Corriger Round Robin

**Fichier**: `policies/roundrobin.c`

**Corrections nécessaires**:

1. **Ajouter détection de duplication** (ligne 28-33)
   ```c
   for (int i = 0; i < n; i++) {
       if (p[i].arrival_time == time && p[i].remaining_time > 0) {
           // Vérifier si déjà en queue
           int already_in_queue = 0;
           for (int j = head; j < tail; j++) {
               if (ready[j] == i) { already_in_queue = 1; break; }
           }
           if (!already_in_queue) ready[tail++] = i;
       }
   }
   ```

2. **Accumuler waiting_time** pendant exécution
   ```c
   // Après exécution du processus
   for (int j = head; j < tail; j++) {
       p[ready[j]].waiting_time += run;
   }
   ```

3. **Clarifier la doc** sur l'ordre des opérations (réémission vs sélection)

---

### Priorité 2: 🔴 HAUTE - Corriger Multilevel Static

**Fichier**: `policies/multilevel.c` OU `Documentation.md`

**Option A - Corriger le code** (Recommandé pour Unix conformité):
```c
int best_prio = INT_MAX;  // Cherche le MINIMUM (Unix style)
for (int i = 0; i < n; i++) {
    if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
        if (procs[i].priority < best_prio) {  // ← Petite valeur
            best_prio = procs[i].priority;
        }
        processes_ready = 1;
    }
}
```

**Option B - Corriger la doc**:
- Changer "Convention: Petite = haute" → "Convention: Grande = haute"
- Changer exemple "1 > 10" → "10 > 1"
- Ajouter note: "⚠️ Convention opposée à Unix standard"

---

### Priorité 3: ⚠️ MOYENNE - Clarifier Round Robin doc

**Fichier**: `Documentation.md` section 3.3

Ajouter après étape 2.1:

> **Note d'Implémentation**: La détection de duplication documentée
> requiert une vérification explicite avant d'ajouter à la queue
> pour éviter que le même processus soit enregistré plusieurs fois.

---

## ✨ Améliorations Suggérées

### 1. Ajouter version details (optionnel)

Chaque section pouvait inclure:
```
**Fichiers implémentation**:
- `policies/algorithm.c` (main logic)
- `src/scheduler.c` (simulation loop)

**Fonctions clés**:
- `void algorithm_simulation(...)`
- `int select_algorithm(...)`
```

### 2. Ajouter exemples d'exécution

Pour chaque algo, inclure exemple de trace:
```
Time  Executing  Ready Queue
----  ---------  -----------
 0    P1         []
 1    P1         [P2]
 2    P2         []
```

### 3. Ajouter tests de validation

Documenter tests unitaires:
- `tests/test_fifo.c`
- `tests/test_priority.c`
- etc.

---

## Fichiers de Référence

| Algorithme | Policy File | Scheduler Loop | Test |
|-----------|-------------|-----------------|------|
| FIFO | `policies/fifo.c` | `scheduler.c:175-220` | `tests/test_fifo.c` |
| Priority | `policies/priority_preemptive.c` | `scheduler.c:240-290` | `tests/test_priority.c` |
| Round Robin | `policies/roundrobin.c` | (inline) | `tests/test_roundrobin.c` |
| SRT | `policies/srt.c` | (inline) | `tests/test_multilevel_dynamic.c` |
| Multilevel | `policies/multilevel.c` | `scheduler.c:310-370` | Tests manuels |
| MLD | `policies/multilevel_dynamic.c` | `scheduler.c:384-470` | Tests manuels |

---

## 📌 Conclusion

**État global**: 2 sections avec problèmes critiques, 4 sections conformes.

**Actions urgentes**:
1. ✅ Corriger Round Robin (3 bugs)
2. ✅ Corriger/clarifier Multilevel convention priorité
3. ⚠️ Améliorer doc avec fichiers/fonctions exactes

**Score de conformité**: 67% actuellement → 100% possible avec corrections.

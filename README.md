# Linux Process Scheduler Simulator

[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Linux](https://img.shields.io/badge/Platform-Linux-green.svg)](https://www.linux.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Multi-task process scheduling simulator for Linux implementing several classic scheduling algorithms.

## 📋 Table of Contents

- [About](#about)
- [Features](#features)
- [Scheduling Algorithms](#scheduling-algorithms)
- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration File Format](#configuration-file-format)
- [Examples](#examples)
- [Testing](#testing)
- [Architecture](#architecture)

## 🎯 About

This project simulates the behavior of a process scheduler by implementing several classic scheduling policies. It allows you to compare the performance of different algorithms in terms of average waiting time and makespan.

## ✨ Features

- **6 scheduling algorithms** implemented
- **Automatic generator** for process configurations
- **Real-time display** of queue status
- **Detailed statistics**: waiting time, makespan, finish time
- **Modular architecture** allowing easy addition of new policies
- **Configuration file support** with comments and validation

## 🔄 Scheduling Algorithms

### 1. **FIFO (First In First Out)**
- Non-preemptive
- Processes are executed in arrival order
- Implementation: `policies/fifo.c`

### 2. **Priority Preemptive**
- Preemptive
- Selection based on priority (configurable: ascending/descending)
- Implementation: `policies/priority_preemptive.c`

### 3. **Round-Robin (RR)**
- Preemptive with configurable time quantum
- Fair CPU distribution among processes
- Implementation: `policies/roundrobin.c`

### 4. **Multilevel Queue**
- Multi-level queues based on priority
- Round-Robin within each level
- Implementation: `policies/multilevel.c`

### 5. **Multilevel Dynamic Queue**
- Multi-level queues with aging mechanism
- Prevention of low-priority process starvation
- Implementation: `policies/multilevel_dynamic.c`

### 6. **SRT (Shortest Remaining Time)**
- Preemptive
- Selection of process with shortest remaining execution time
- Implementation: `policies/srt.c`

## 📁 Project Structure

```
Projet-Ordonnancement-Linux/
├── config/                     # Configuration files
│   ├── sample_config.txt      # Sample configuration
│   └── *.txt                  # Generated configurations
├── include/                    # Header files
│   ├── process.h              # Process structure
│   ├── scheduler.h            # Scheduler interface
│   ├── parser.h               # Configuration parser
│   ├── utils.h                # Utility functions
│   └── generate_config.h      # Config generator
├── policies/                   # Algorithm implementations
│   ├── fifo.c                 # FIFO
│   ├── priority_preemptive.c  # Preemptive priority
│   ├── roundrobin.c           # Round-Robin
│   ├── multilevel.c           # Multi-level queues
│   ├── multilevel_dynamic.c   # Dynamic multi-level
│   └── srt.c                  # Shortest Remaining Time
├── src/                        # Main source code
│   ├── main.c                 # Entry point
│   ├── scheduler.c            # Scheduler logic
│   ├── parser.c               # File parser
│   ├── utils.c                # Utilities
│   └── generate_config.c      # Configuration generator
├── tests/                      # Unit tests
│   ├── test_fifo.c
│   ├── test_priority.c
│   ├── test_roundrobin.c
│   ├── test_multilevel.c
│   └── test_multilevel_dynamic.c
├── build/                      # Object files (generated)
├── Makefile                    # Build script
└── README.md                   # This file
```

## 🔧 Prerequisites

- **Compiler**: GCC (GNU Compiler Collection)
- **System**: Linux (Ubuntu, Debian, Fedora, etc.)
- **Standard**: C11 or higher

## 📦 Installation

1. **Clone the repository** (or download the sources):
```bash
git clone <repository-url>
cd Projet-Ordonnancement-Linux-arij-dev
```

2. **Compile the project**:
```bash
make
```

This will create the `ordonnanceur` executable at the project root.

3. **Clean compiled files** (optional):
```bash
make clean
```

## 🚀 Usage

### Basic Launch

```bash
./ordonnanceur
```

At startup, two options are available:

1. **Automatically generate a configuration** (default)
   - The program asks for generation parameters
   - A configuration file is created in `config/`

2. **Use an existing configuration file**
   - Enter the file path (e.g., `config/sample_config.txt`)

### Simulation Workflow

1. **Loading** the configuration
2. **Display** of loaded processes
3. **Selection** of the scheduling policy
4. **Simulation** with real-time display
5. **Final statistics**:
   - Average waiting time
   - Makespan (total execution time)
   - Finish time for each process

## 📄 Configuration File Format

Configuration files follow this format:

```
# Comment: process example
# Format: name arrival duration priority

P1 0 20 3
P2 10 12 2
P3 5 15 1
P4 20 18 4
```

**Rules**:
- Empty lines are ignored
- Comments start with `#`
- Each valid line: `NAME ARRIVAL DURATION PRIORITY`
- `ARRIVAL`: arrival time (≥ 0)
- `DURATION`: required execution time (> 0)
- `PRIORITY`: process priority (integer)

## 💡 Examples

### Automatic Configuration Generation

```
=== Scheduler Project ===
1) Generate configuration file automatically  (default)
2) Use an existing configuration file
Your choice (press ENTER for default): [ENTER]

➡ Generating configuration file: config/sample_config_20251205_143025.txt

=== Automatic Process Configuration Generator ===
Number of processes        : 5
Maximum arrival time       : 20
Minimum priority           : 1
Maximum priority           : 5
Maximum execution time     : 15

✔ File 'config/sample_config_20251205_143025.txt' generated successfully.
```

### Policy Selection

```
=== Choose a policy ===
0 - fifo.c
1 - multilevel.c
2 - multilevel_dynamic.c
3 - priority_preemptive.c
4 - roundrobin.c
5 - srt.c
Choice [Enter → FIFO]: 4
```

### Simulation Output (Round-Robin)

```
=== ROUND-ROBIN (Quantum = 3) ===
Time  Executing     Ready Queue
----  -----------  --------------------------------
   0  P1          []
   3  P2          [P3:15]
   6  P3          [P1:17]
   9  P1          [P2:9, P3:12]
  12  P2          [P3:9, P1:14]
...

FINAL STATISTICS
Name  Arrival  Exec  Finish  Wait
----  -------  ----  ------  ----
P1         0    20      45    25
P2        10    12      38    16
P3         5    15      42    22

Average Wait Time: 21.00
Makespan: 45
```

## 🧪 Testing

Unit tests are available for each algorithm:

```bash
# Compile and run a specific test
# Fifo test
gcc -Iinclude tests/test_fifo.c policies/fifo.c -o test_unit && ./test_unit

# RoundRobin test
gcc -Iinclude tests/test_roundrobin.c policies/roundrobin.c -o test_unit && ./test_unit

# Priority Preemptive test
gcc -Iinclude tests/test_priority.c policies/priority_preemptive.c -o test_unit && ./test_unit
```

Tests display:
- Test process table
- Gantt chart
- Queue status at each time instant
- Final statistics

## 🏗️ Architecture

### Process Structure

```c
struct process {
    char name[NAME_LEN];      // Process name
    int arrival_time;         // Arrival time
    int exec_time;            // Total execution duration
    int priority;             // Static priority
    int remaining_time;       // Remaining time
    int waiting_time;         // Accumulated waiting time
    int status;               // State (READY, RUNNING, BLOCKED, ZOMBIE)
    int end_time;             // Finish time
    int wait_time;            // For dynamic aging
};
```

### Policy Interface

Each policy implements a selection function:

```c
int policy_scheduler(struct process *procs, int n, int time, int current, int param);
```

**Parameters**:
- `procs`: process array
- `n`: number of processes
- `time`: current time
- `current`: index of current process (-1 if none)
- `param`: specific parameter (mode, quantum, etc.)

**Return**: index of process to execute, or -1 if IDLE

### Dynamic Loading

The system automatically loads available policies by scanning the `policies/` directory at startup.

## 📊 Metrics

The simulator calculates for each execution:

- **Waiting time**: `wait_time = end_time - arrival_time - exec_time`
- **Average waiting time**: `avg_wait = sum(wait_time) / n`
- **Makespan**: total execution time (last process finished)
- **Response time**: time between arrival and first CPU access

## 🤝 Contributing

Contributions are welcome! To add a new policy:

1. Create `policies/new_policy.c`
2. Implement the selection function
3. Add the prototype in `include/scheduler.h`
4. Add the simulation in `src/scheduler.c`
5. Create a test in `tests/`

## 📝 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## 👥 Author

Project developed as part of the Operating Systems course - 1st year Engineering.

---

**Note**: This project is for educational and demonstration purposes. It simulates scheduler behavior but does not represent a real system implementation.

# Architecture Documentation – Ordonnanceur Linux

## Table of Contents

1. [System Architecture Overview](#system-architecture-overview)
2. [Component Design](#component-design)
3. [Data Structures](#data-structures)
4. [Algorithm Implementations](#algorithm-implementations)
5. [API Design & Protocols](#api-design--protocols)
6. [Frontend Architecture](#frontend-architecture)
7. [Backend Architecture](#backend-architecture)
8. [Inter-Process Communication](#inter-process-communication)
9. [Scalability & Performance](#scalability--performance)
10. [Security Considerations](#security-considerations)

---

## System Architecture Overview

### High-Level Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                                                              │
│                    USER INTERFACE LAYER                      │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  Web Browser (React Components)                        │ │
│  │  - FileUpload Component                                │ │
│  │  - AlgorithmSelector Component                         │ │
│  │  - ResultsDisplay Component                            │ │
│  │  - GanttChart (Recharts)                               │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                              │
└────────────────────┬─────────────────────────────────────────┘
                     │
                HTTP │ POST
            /api/    │
            parse-   │     /api/schedule
            config   │
                     │
┌────────────────────▼─────────────────────────────────────────┐
│                                                              │
│              APPLICATION LAYER (Next.js)                    │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  Route Handlers                                        │ │
│  │  - /api/parse-config/route.ts                          │ │
│  │  - /api/schedule/route.ts                              │ │
│  │  - Error handling, validation                          │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                              │
└────────────────────┬─────────────────────────────────────────┘
                     │
             spawn() │ child_process
                     │
┌────────────────────▼─────────────────────────────────────────┐
│                                                              │
│           COMPUTATION LAYER (C Backend)                     │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  Parsing Module                                        │ │
│  │  - Read config files                                   │ │
│  │  - Validate process definitions                        │ │
│  │  - Build process array (struct process[])             │ │
│  └────────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  Scheduling Engine                                     │ │
│  │  - Algorithm selection                                 │ │
│  │  - Simulation time loop                                │ │
│  │  - Gantt segment generation                            │ │
│  │  - Metrics calculation                                 │ │
│  └────────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  Algorithm Implementations                             │ │
│  │  - fifo.c, priority_preemptive.c, roundrobin.c, etc.  │ │
│  │  - Each implements selection_function(...)             │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                              │
└────────────────────┬─────────────────────────────────────────┘
                     │
            JSON    │ stdout
           output   │
                     │
┌────────────────────▼─────────────────────────────────────────┐
│                                                              │
│              DATA LAYER (Results)                           │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  JSON Result Object                                    │ │
│  │  - Gantt segments (process, time range, type)          │ │
│  │  - Process statistics (wait, turnaround, completion)   │ │
│  │  - System metrics (avg wait, CPU util, makespan)       │ │
│  │  - Queue timeline (ready queue evolution)              │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### Architecture Patterns

**Pattern 1: Layered Architecture**
- Separation of concerns (UI ↔ API ↔ Simulation ↔ Algorithms)
- Each layer independent and testable

**Pattern 2: Process-Agnostic**
- Backend C process is stateless
- Each invocation is independent
- No shared state between simulations

**Pattern 3: Pub-Sub (Frontend)**
- React hooks (useState, useEffect) manage state
- Components re-render on data changes
- Minimal prop drilling

---

## Component Design

### Frontend Components (React/TypeScript)

#### Page Component (`app/page.tsx`)

**Purpose:** Main container orchestrating all interactions

**State:**
```typescript
- processes: Process[]                    // Loaded process definitions
- algorithmConfig: AlgorithmConfig        // Selected algorithm + params
- results: SchedulingResult | null        // Simulation results
- isRunning: boolean                      // Simulation in progress
- showProcessTable: boolean               // Details visibility
- currentFileName: string                 // Track loaded file
- loadingDefault: boolean                 // Initial load flag
```

**Responsibilities:**
1. Load default config on mount (useEffect)
2. Handle file uploads/generation
3. Launch API requests to backend
4. Pass results to ResultsDisplay
5. Manage UI state (loading, errors)

**Key Methods:**
```typescript
handleGenerateFile(processes)      // Update processes from generator
handleChooseFile(file)             // Handle file upload
handleLaunchSimulation()           // POST to /api/schedule
handleToggleDetails()              // Show/hide statistics table
```

#### ResultsDisplay Component (`components/results-display.tsx`)

**Purpose:** Comprehensive results visualization

**Props:**
```typescript
{
  results: SchedulingResult | null
  isRunning: boolean
  onRunningChange: (running: boolean) => void
  currentTime?: number
  onTimeChange?: (time: number) => void
}
```

**Subcomponents:**
1. **GanttChart** (Recharts)
   - Y-axis: Process names
   - X-axis: Time units
   - Segments: Execution (blue) + Waiting (grey)
   - Timeline slider overlay

2. **Statistics Table**
   - Per-process metrics
   - Expandable accordion

3. **Charts Section**
   - Pie chart: CPU time distribution
   - Bar chart: Wait vs turnaround times
   - Line chart: Queue depth timeline

4. **Controls**
   - Play/Pause buttons
   - Step forward/backward
   - Speed selector

**Key Methods:**
```typescript
buildTicks()              // Generate dynamic timeline ticks
calculateMaxTime()        // Find simulation end time
getCurrentSegments()      // Filter Gantt by current time
updateQueueTimeline()     // Animate queue depth
```

#### AlgorithmSelector Component (`components/algorithm-selector.tsx`)

**Purpose:** Algorithm and parameter selection

**Props:**
```typescript
{
  config: AlgorithmConfig
  onConfigChange: (config: AlgorithmConfig) => void
  isRunning: boolean
}
```

**Controls:**
- Dropdown: Algorithm selection (6 options)
- Input: Quantum (for RR/Multilevel)
- Toggle: Priority order (ascending/descending)
- Submit: Launch simulation button

#### FileGenerationDialog Component (`components/file-generation-dialog.tsx`)

**Purpose:** Process generation UI

**Features:**
- Number of processes slider
- Execution time range (min/max)
- Arrival time range
- Priority range
- Preview generated processes
- Save/Cancel actions

---

### Backend Components (C)

#### Parser Module (`src/parser.c`)

**Key Structures:**
```c
struct process {
  char name[NAME_LEN];           // P1, P2, etc.
  int arrival_time;             // When process becomes ready
  int exec_time;                // Total execution time
  int priority;                 // Priority level (higher = more important)
  int remaining_time;           // Remaining execution time (updated)
  int waiting_time;             // Accumulated wait time
  int end_time;                 // Completion time (-1 if incomplete)
};
```

**Functions:**
```c
int parse_config_file(const char *path, struct process **out, int *out_n)
// - Open and read file
// - Parse lines: NAME ARRIVAL EXECUTION PRIORITY
// - Skip comments and empty lines
// - Return process array

void sort_by_arrival(struct process *procs, int n)
// - Sort processes by arrival time (used in simulations)
// - Important for FIFO and other algorithms
```

**Error Handling:**
```c
- Returns -1 on file error
- Validates each field
- Skips invalid lines (logs but continues)
- Handles memory allocation failures
```

#### Scheduler Module (`src/scheduler.c`)

**Purpose:** Orchestrate simulations and output formatting

**Key Functions:**

```c
void fifo_simulation(struct process *p, int n)
// Execute FIFO scheduling
// Print ASCII table output

void priority_simulation(struct process *p, int n, int prio_mode)
// Execute priority scheduling
// prio_mode: 0=ascending, 1=descending

void rr_simulation(struct process *p, int n)
// Prompt for quantum, execute RR

void multilevel_simulation(struct process *procs, int n, int quantum)
// Execute static multilevel with RR at each level

void multilevel_dynamic_simulation(struct process *procs, int n, int quantum)
// Execute dynamic multilevel with aging

void print_json_result(struct simulation_result *res)
// Output JSON format (for API mode)
```

**Simulation Loop Pattern:**
```c
while (finished < n) {
    // 1. Select next process to execute
    int idx = algorithm_select(procs, n, time, current, ...);
    
    if (idx == -1) {
        // CPU IDLE
        time++;
        continue;
    }
    
    // 2. Execute process for 1 time unit
    procs[idx].remaining_time--;
    procs[idx].waiting_time++; // For non-running processes
    
    // 3. Check completion
    if (procs[idx].remaining_time == 0) {
        procs[idx].end_time = time + 1;
        finished++;
    }
    
    // 4. Print/record state
    // (output to stdout or build JSON)
    
    time++;
}
```

#### Algorithm Implementations

Each algorithm implements a selection function:

```c
// fifo.c
int fifo_scheduler(struct process *procs, int n, int time, int current, int prio_mode)
// Returns index of next process to execute
// -1 if none ready

// priority_preemptive.c
int priority_preemptive(struct process *procs, int n, int time, int current, int prio_mode)
// Always returns highest priority ready process

// roundrobin.c
// No selection function; uses queue-based approach

// srt.c
int srt_scheduler(struct process *procs, int n, int time, int current, int prio_mode)
// Returns process with minimum remaining time

// multilevel.c
int select_multilevel(struct process *procs, int n, int time, int current, int quantum_expired)
// Select from highest priority level with ready processes

// multilevel_dynamic.c
int select_multilevel_dynamic(struct process *procs, int n, int time, int current, int quantum_expired)
// Like multilevel but with aging mechanism
```

---

## Data Structures

### Core Process Structure

```c
struct process {
  char name[16];           // Process identifier
  int arrival_time;        // Time when process enters ready queue
  int exec_time;          // Total execution time needed
  int priority;           // Priority level (algorithm-dependent)
  int remaining_time;     // Time still needed (decremented during execution)
  int waiting_time;       // Total time spent waiting
  int end_time;          // When process completes (-1 if not done)
};
```

### API Response Structures (TypeScript)

```typescript
// Frontend request type
interface ScheduleRequest {
  processes: Process[]
  config: AlgorithmConfig
}

// Frontend response type
interface SchedulingResult {
  algorithm: string
  processes: ProcessResult[]
  metrics: ScheduleMetrics
  ganttChart: GanttSegment[]
  queueTimeline: QueueState[]
}

interface ProcessResult {
  id: string
  arrivalTime: number
  executionTime: number
  completionTime: number
  waitingTime: number
  turnaroundTime: number
}

interface ScheduleMetrics {
  averageWaitTime: number
  averageTurnaroundTime: number
  cpuUtilization: number
  makespan: number
}

interface GanttSegment {
  processId: string
  startTime: number
  endTime: number
  type: 'execution' | 'waiting'
}

interface QueueState {
  time: number
  queue: string[]
  executing: string | null
}
```

---

## Algorithm Implementations

### FIFO Implementation

```c
int fifo_scheduler(struct process *procs, int n, int time, int current, int prio_mode) {
    int best_index = -1;
    int earliest_arrival = INT_MAX;

    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            if (procs[i].arrival_time < earliest_arrival) {
                earliest_arrival = procs[i].arrival_time;
                best_index = i;
            }
        }
    }

    return best_index;  // -1 if none ready
}
```

**Key Points:**
- Non-preemptive (once selected, runs to completion)
- Selects earliest arriving ready process
- O(n) per time unit

---

### Priority Preemptive Implementation

```c
int priority_preemptive(struct process *procs, int n, int time, int current, int prio_mode) {
    int best_prio = -1;
    int best_index = -1;

    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            int prio_val = prio_mode ? procs[i].priority : -procs[i].priority;
            
            if (prio_val > best_prio) {
                best_prio = prio_val;
                best_index = i;
            }
        }
    }

    return best_index;
}
```

**Key Points:**
- Preemptive: checks every time unit
- prio_mode=1: higher number = higher priority
- prio_mode=0: lower number = higher priority

---

### Round Robin Implementation

```c
void round_robin(struct process *procs, int n, int quantum) {
    int time = 0;
    int completed = 0;
    int ready[100];  // Queue of ready process indices
    int head = 0, tail = 0;

    while (completed < n) {
        // Add newly arrived processes
        for (int i = 0; i < n; i++) {
            if (procs[i].arrival_time == time && procs[i].remaining_time > 0) {
                ready[tail++] = i;
            }
        }

        if (head == tail) {
            // No ready process: IDLE
            time++;
            continue;
        }

        // Get front of queue
        int idx = ready[head];
        
        // Execute for quantum or until completion
        int exec_time = (procs[idx].remaining_time < quantum) 
                        ? procs[idx].remaining_time 
                        : quantum;

        procs[idx].remaining_time -= exec_time;
        time += exec_time;

        if (procs[idx].remaining_time > 0) {
            // Not done: return to queue
            ready[head] = -1;  // Mark as removed
            head = (head + 1) % n;
            ready[tail++] = idx;
        } else {
            // Completed
            procs[idx].end_time = time;
            completed++;
            head = (head + 1) % n;
        }
    }
}
```

**Key Points:**
- Circular queue of ready processes
- Each gets quantum time units
- If not done, returns to back of queue

---

### Multilevel Implementation

```c
int select_multilevel(struct process *procs, int n, int time, int current, int quantum_expired) {
    // Find highest priority level with ready processes
    int best_prio = -1;
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            if (procs[i].priority > best_prio) {
                best_prio = procs[i].priority;
            }
        }
    }

    if (best_prio == -1) return -1;  // All idle

    // Round Robin among processes at best_prio level
    // Use queue or round-robin logic here
    for (int i = 0; i < n; i++) {
        if (procs[i].priority == best_prio && 
            procs[i].arrival_time <= time && 
            procs[i].remaining_time > 0) {
            return i;
        }
    }

    return -1;
}
```

---

### Multilevel Dynamic (with Aging)

```c
int select_multilevel_dynamic(struct process *procs, int n, int time, int current, int quantum_expired) {
    // Dynamic priority adjustment based on waiting time
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            int wait_time = time - procs[i].arrival_time - (procs[i].exec_time - procs[i].remaining_time);
            
            // Aging: increase priority if waiting too long
            if (wait_time > AGING_THRESHOLD) {
                procs[i].priority = min(procs[i].priority + 1, MAX_PRIORITY);
            }
        }
    }

    // Now select same as multilevel
    return select_multilevel(procs, n, time, current, quantum_expired);
}
```

---

## API Design & Protocols

### REST Endpoints

#### POST /api/parse-config

**Headers:**
```
Content-Type: application/json  OR  multipart/form-data
```

**Request Body (JSON):**
```json
{
  "defaultFile": "sample_config.txt"
}
```

**Request Body (FormData):**
```
--boundary
Content-Disposition: form-data; name="file"; filename="config.txt"
Content-Type: text/plain

[file content]
--boundary--
```

**Response (200 OK):**
```json
[
  {
    "id": "P1",
    "arrivalTime": 0,
    "executionTime": 5,
    "priority": 1,
    "remainingTime": 5,
    "waitingTime": 0,
    "completionTime": null
  }
]
```

**Error Responses:**
- 400: Invalid format or missing fields
- 500: File system or backend error

#### POST /api/schedule

**Request:**
```json
{
  "processes": [
    {
      "id": "P1",
      "arrivalTime": 0,
      "executionTime": 5,
      "priority": 1
    }
  ],
  "config": {
    "algorithm": "roundrobin",
    "quantum": 2,
    "priorityOrder": "descending"
  }
}
```

**Response (200 OK):**
```json
{
  "algorithm": "roundrobin",
  "processes": [
    {
      "id": "P1",
      "arrivalTime": 0,
      "executionTime": 5,
      "completionTime": 10,
      "waitingTime": 5,
      "turnaroundTime": 10
    }
  ],
  "metrics": {
    "averageWaitTime": 3.5,
    "averageTurnaroundTime": 7.5,
    "cpuUtilization": 0.8,
    "makespan": 10
  },
  "ganttChart": [
    {
      "processId": "P1",
      "startTime": 0,
      "endTime": 2,
      "type": "execution"
    }
  ],
  "queueTimeline": [
    {
      "time": 0,
      "queue": ["P1"],
      "executing": "P1"
    }
  ]
}
```

### Backend Command-Line Interface

```bash
# Parse config file (output JSON)
./ordonnanceur --parse-config config/sample_config.txt

# API mode (output JSON)
./ordonnanceur --api \
  --config config/sample_config.txt \
  --algo roundrobin \
  --quantum 2 \
  --prio-order desc

# Interactive CLI mode
./ordonnanceur config/sample_config.txt
```

### C Backend Output Format

**JSON Output (--api mode):**
```json
{
  "algorithm": "fifo",
  "gantt": [
    {"process": "P1", "start": 0, "end": 5, "type": "exec"},
    {"process": "P2", "start": 5, "end": 8, "type": "exec"}
  ],
  "processes": [
    {"name": "P1", "arrival": 0, "execution": 5, "wait": 0, "turnaround": 5, "end": 5},
    {"name": "P2", "arrival": 2, "execution": 3, "wait": 3, "turnaround": 6, "end": 8}
  ],
  "metrics": {
    "avg_wait": 1.5,
    "avg_turnaround": 5.5,
    "cpu_util": 0.95,
    "makespan": 8
  }
}
```

---

## Frontend Architecture

### State Management

**No external state library** – Uses React's built-in hooks:

```typescript
// Page-level state
const [processes, setProcesses] = useState<Process[]>([])
const [results, setResults] = useState<SchedulingResult | null>(null)
const [isRunning, setIsRunning] = useState(false)

// Component-level state
const [currentTime, setCurrentTime] = useState(0)
const [isPlaying, setIsPlaying] = useState(false)
```

**State Flow:**
```
Page Component (root state)
    ↓
    ├→ ResultsDisplay (read results, currentTime)
    │   └→ GanttChart (render based on props)
    │
    ├→ AlgorithmSelector (read config, emit updates)
    │
    └→ FileGenerationDialog (emit new processes)
```

### Component Lifecycle

**Initial Load:**
```
1. App mounts
2. useEffect triggers (empty dependency)
3. loadDefaultConfig() runs
4. setProcesses(parsed_data)
5. setCurrentFileName("sample_config.txt")
6. setLoadingDefault(false)
```

**User Launches Simulation:**
```
1. User clicks "Lancer la Simulation"
2. handleLaunchSimulation() called
3. POST /api/schedule with processes + config
4. setIsRunning(true)
5. Response received
6. setResults(data)
7. setIsRunning(false)
```

**Timeline Animation:**
```
1. User clicks Play
2. setIsPlaying(true)
3. requestAnimationFrame loop starts
4. Increment currentTime each frame
5. ResultsDisplay re-renders with new time
6. GanttChart filters segments by time
7. When currentTime >= maxTime, stop
```

### Responsive Design

**Tailwind CSS with mobile-first approach:**
```typescript
// example: card padding
className="p-4 md:p-6 lg:p-8"

// Grid layout
className="grid grid-cols-1 md:grid-cols-2 gap-4"

// Visibility
className="hidden lg:block"  // Hide on small screens
```

---

## Backend Architecture

### C Project Structure

**Compilation Flow:**
```
Makefile
  ├─ Compiles src/*.c → build/*.o
  ├─ Compiles policies/*.c → build/*.o
  ├─ Links all .o → ordonnanceur (executable)
  └─ Includes headers from include/
```

**Build Flags:**
```makefile
CFLAGS = -Wall -Wextra -std=c11 -I$(INC_DIR)
# -Wall: Enable all warnings
# -Wextra: Extra warnings
# -std=c11: C11 standard
# -I: Include directory path
```

### Memory Management

**No manual memory allocation** (mostly static arrays):
```c
struct process procs[100];  // Static array, stack allocated

// When necessary, dynamic allocation:
struct process *procs = malloc(n * sizeof(struct process));
// ... use it
free(procs);
```

**Key Principle:** Each invocation is independent; no state persists between runs.

### Initialization

**Program Entry (`main.c`):**
```c
int main(int argc, char **argv) {
    // 1. Parse arguments
    // 2. Determine mode (direct file, API, parse-only)
    // 3. Load and parse config file
    // 4. Execute appropriate scheduler function
    // 5. Output results and exit
    return 0;
}
```

---

## Inter-Process Communication

### Process Spawning (Node.js ↔ C Binary)

**From Next.js Route Handler:**
```typescript
const child: ChildProcessWithoutNullStreams = spawn(binaryPath, args, {
  stdio: ["ignore", "pipe", "pipe"]
})

// Capture stdout
child.stdout.on("data", (data) => {
  stdout += data.toString()
})

// Capture stderr
child.stderr.on("data", (data) => {
  stderr += data.toString()
})

// Wait for completion
child.on("close", (code) => {
  // Parse JSON from stdout
  const result = JSON.parse(stdout)
  response.json(result)
})
```

### Temporary File Handling

**Why needed:**
- C backend reads config from files (not stdin)
- Need to convert FormData/Object to file

**Process:**
```typescript
1. Receive FormData or JSON from frontend
2. Write to temp file in /tmp/
3. Spawn C binary with path to temp file
4. Wait for completion
5. Delete temp file
6. Return results to frontend
```

**Code:**
```typescript
const tmpPath = path.join(os.tmpdir(), `config_${Date.now()}.txt`)
await fs.writeFile(tmpPath, configContent)

// ... run binary ...

await fs.unlink(tmpPath)  // Cleanup
```

### Output Protocol

**Backend outputs JSON to stdout:**
```c
printf("{");
printf("  \"algorithm\": \"%s\",", algo_name);
printf("  \"processes\": [");
// ... output process array
printf("  ]");
printf("}");
```

**Frontend parses:**
```typescript
const result = JSON.parse(stdout)
setResults(result)
```

---

## Scalability & Performance

### Computational Complexity

| Algorithm | Time | Space | Notes |
|-----------|------|-------|-------|
| FIFO | O(n²) | O(n) | Linear search per time unit |
| Priority | O(n²) | O(n) | Linear search per selection |
| RR | O(n²) | O(n) | Queue operations per time unit |
| SRT | O(n²) | O(n) | Linear search for minimum |
| Multilevel | O(n²) | O(n) | Linear search per level |
| Multilevel Dyn | O(n² log n) | O(n) | Aging requires comparisons |

### Limits & Bottlenecks

**Process Count Limits:**
```
- Hard limit: 100-1000 processes (static arrays)
- Practical limit: 500 processes (UI rendering)
- Web response time: < 5s for 500 processes
```

**Timeline Length:**
```
- Typical: 0-100 time units
- Max practical: 1000 time units
- Rendering thousands of Gantt segments: ~1-2s
```

### Optimization Strategies

**Frontend:**
1. Memoize expensive computations
2. Virtualize long lists (if needed)
3. Lazy-load chart libraries

**Backend:**
1. Use static arrays (stack > heap)
2. Minimize realloc operations
3. Early termination loops

**Example - Optimized Selection:**
```c
// Instead of:
int best = -1, best_prio = -1;
for (int i = 0; i < n; i++) {
    if (procs[i].priority > best_prio) {
        best_prio = procs[i].priority;
        best = i;
    }
}

// Cache best_prio to avoid repeated comparisons
```

---

## Security Considerations

### Input Validation

**Frontend:**
```typescript
// Validate file size
if (file.size > 1_000_000) {
  throw new Error("File too large (max 1MB)")
}

// Validate file type
if (!file.type.includes("text")) {
  throw new Error("Must be text file")
}

// Sanitize process names
const sanitizedName = name.replace(/[^a-zA-Z0-9_]/g, "")
```

**Backend:**
```c
// Validate array bounds
if (n >= MAX_PROCESSES) {
    fprintf(stderr, "Too many processes\n");
    return -1;
}

// Validate arrival/execution times
if (arrival_time < 0 || exec_time <= 0) {
    fprintf(stderr, "Invalid time values\n");
    continue;
}

// Buffer overflow protection
strncpy(arr[n].name, tokens[0], NAME_LEN - 1);
arr[n].name[NAME_LEN - 1] = '\0';  // Ensure null termination
```

### Path Traversal Prevention

```typescript
// ✓ Safe: Use basename, restricted directory
const safePath = path.join(SAFE_CONFIG_DIR, path.basename(filename))

// ✗ Unsafe: Direct user input
const unsafePath = userInput  // Could be "../../etc/passwd"
```

### Resource Limits

```typescript
// Timeout for long-running simulations
const timeout = setTimeout(() => {
  child.kill()
  response.status(504).json({ error: "Simulation timeout" })
}, 30_000)  // 30 seconds

// Memory limit
child.on("error", (err) => {
  if (err.code === "ENOMEM") {
    response.status(507).json({ error: "Out of memory" })
  }
})
```

### Error Messages

**Do Not Reveal:**
- System paths
- Software versions
- Internal implementation details

**Example:**
```typescript
// ✗ Bad
throw new Error(`Binary at /home/user/ordonnanceur not found`)

// ✓ Good
throw new Error(`Scheduler backend not available. Ensure 'make' was run.`)
```

---

## Testing Strategy

### Unit Tests (C Backend)

```bash
# Test individual algorithms
./tests/test_fifo
./tests/test_roundrobin
./tests/test_priority
./tests/test_multilevel
./tests/test_multilevel_dynamic

# Test parser
./tests/test_parser
```

### Integration Tests (Full Flow)

```bash
# CLI integration test
./ordonnanceur config/sample_config.txt

# API integration test (with curl)
curl -X POST http://localhost:3000/api/schedule \
  -H "Content-Type: application/json" \
  -d '{...}'
```

### Frontend Tests

```bash
# Build verification
pnpm build

# Lint check
pnpm lint

# Manual testing
pnpm dev  # Open http://localhost:3000
```

---

**Last Updated:** December 2024  
**Version:** 1.0.0  
**Architecture Style:** Layered (UI → API → Simulation → Algorithms)


# API Reference – Ordonnanceur Linux

## Table of Contents

1. [Overview](#overview)
2. [Authentication](#authentication)
3. [Request/Response Format](#requestresponse-format)
4. [Endpoints](#endpoints)
5. [Data Models](#data-models)
6. [Error Handling](#error-handling)
7. [Rate Limiting](#rate-limiting)
8. [Code Examples](#code-examples)
9. [Webhooks](#webhooks)
10. [Pagination](#pagination)

---

## Overview

### API Specification

| Property | Value |
|----------|-------|
| **Base URL** | `http://localhost:3000/api` |
| **Version** | 1.0.0 |
| **Protocol** | HTTP/HTTPS |
| **Format** | JSON (application/json) |
| **Authentication** | None (public) |
| **Rate Limit** | None (local deployment) |

### API Capabilities

- **Parse configuration files** – Validate and convert process definitions
- **Execute scheduling simulations** – Run selected algorithm on process set
- **Return detailed metrics** – Performance statistics and visualization data
- **Support multiple formats** – JSON input/output with multipart file uploads

---

## Authentication

### Current Policy

✅ **Public/No Authentication Required**

The API is designed for local or trusted network deployment. No authentication headers are required.

### Future Security

For production cloud deployment, consider:

```typescript
// Authorization header (Bearer token)
Authorization: Bearer <jwt_token>

// API key header
X-API-Key: <api_key>

// Custom header with HMAC signature
X-Signature: sha256=<hmac_signature>
```

---

## Request/Response Format

### Content Types

**Requests:**
- `application/json` – JSON body
- `multipart/form-data` – File uploads

**Responses:**
- `application/json` – Standard response
- `text/plain` – Error messages (rare)

### HTTP Methods

| Method | Purpose |
|--------|---------|
| **POST** | Create/execute operations (parse config, run simulation) |
| **GET** | Not implemented (all operations are stateless POST) |

### Headers

**Request Headers (Typical):**
```
Content-Type: application/json
Accept: application/json
User-Agent: Mozilla/5.0 (custom client)
```

**Response Headers (Standard):**
```
Content-Type: application/json
Content-Length: <bytes>
X-Response-Time: <milliseconds>
```

---

## Endpoints

### POST /api/parse-config

**Purpose:** Parse and validate a configuration file

#### Request (Option 1: Load Default File)

```http
POST /api/parse-config HTTP/1.1
Host: localhost:3000
Content-Type: application/json

{
  "defaultFile": "sample_config.txt"
}
```

#### Request (Option 2: Upload File)

```http
POST /api/parse-config HTTP/1.1
Host: localhost:3000
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary

------WebKitFormBoundary
Content-Disposition: form-data; name="file"; filename="my_config.txt"
Content-Type: text/plain

P1 0 5 1
P2 2 3 2
P3 4 2 2
------WebKitFormBoundary--
```

#### Response (Success 200)

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
  },
  {
    "id": "P2",
    "arrivalTime": 2,
    "executionTime": 3,
    "priority": 2,
    "remainingTime": 3,
    "waitingTime": 0,
    "completionTime": null
  },
  {
    "id": "P3",
    "arrivalTime": 4,
    "executionTime": 2,
    "priority": 2,
    "remainingTime": 2,
    "waitingTime": 0,
    "completionTime": null
  }
]
```

#### Response (Error 400 - Bad Request)

```json
{
  "error": "Invalid file format. Expected: NAME ARRIVAL EXECUTION PRIORITY"
}
```

#### Response (Error 500 - Server Error)

```json
{
  "error": "Failed to parse configuration file. Backend binary not found."
}
```

---

### POST /api/schedule

**Purpose:** Execute a scheduling algorithm and return detailed results

#### Request

```http
POST /api/schedule HTTP/1.1
Host: localhost:3000
Content-Type: application/json

{
  "processes": [
    {
      "id": "P1",
      "arrivalTime": 0,
      "executionTime": 5,
      "priority": 1
    },
    {
      "id": "P2",
      "arrivalTime": 2,
      "executionTime": 3,
      "priority": 2
    },
    {
      "id": "P3",
      "arrivalTime": 4,
      "executionTime": 2,
      "priority": 2
    }
  ],
  "config": {
    "algorithm": "roundrobin",
    "quantum": 2,
    "priorityOrder": "descending"
  }
}
```

#### Request Fields

```typescript
interface ScheduleRequest {
  processes: Array<{
    id: string              // Process identifier (e.g., "P1")
    arrivalTime: number     // Time when process becomes ready (>= 0)
    executionTime: number   // Total time units needed (> 0)
    priority: number        // Priority level (1-10, higher = more urgent)
  }>
  config: {
    algorithm: string       // fifo | priority | roundrobin | srt | multilevel | multilevel_dynamic
    quantum?: number        // Time slice (required for RR, multilevel variants)
    priorityOrder?: string  // ascending | descending (default: descending)
  }
}
```

#### Response (Success 200)

```json
{
  "algorithm": "roundrobin",
  "processes": [
    {
      "id": "P1",
      "arrivalTime": 0,
      "executionTime": 5,
      "completionTime": 9,
      "waitingTime": 4,
      "turnaroundTime": 9
    },
    {
      "id": "P2",
      "arrivalTime": 2,
      "executionTime": 3,
      "completionTime": 7,
      "waitingTime": 2,
      "turnaroundTime": 5
    },
    {
      "id": "P3",
      "arrivalTime": 4,
      "executionTime": 2,
      "completionTime": 10,
      "waitingTime": 4,
      "turnaroundTime": 6
    }
  ],
  "metrics": {
    "averageWaitTime": 3.33,
    "averageTurnaroundTime": 6.67,
    "cpuUtilization": 1.0,
    "makespan": 10
  },
  "ganttChart": [
    {
      "processId": "P1",
      "startTime": 0,
      "endTime": 2,
      "type": "execution"
    },
    {
      "processId": "P1",
      "startTime": 2,
      "endTime": 3,
      "type": "waiting"
    },
    {
      "processId": "P2",
      "startTime": 3,
      "endTime": 4,
      "type": "waiting"
    },
    {
      "processId": "P2",
      "startTime": 4,
      "endTime": 6,
      "type": "execution"
    },
    {
      "processId": "P3",
      "startTime": 6,
      "endTime": 8,
      "type": "execution"
    },
    {
      "processId": "P1",
      "startTime": 8,
      "endTime": 9,
      "type": "execution"
    },
    {
      "processId": "P2",
      "startTime": 9,
      "endTime": 10,
      "type": "execution"
    }
  ],
  "queueTimeline": [
    {
      "time": 0,
      "queue": ["P1"],
      "executing": "P1"
    },
    {
      "time": 2,
      "queue": ["P1", "P2"],
      "executing": "P2"
    },
    {
      "time": 4,
      "queue": ["P1", "P3"],
      "executing": "P3"
    },
    {
      "time": 6,
      "queue": ["P1"],
      "executing": "P1"
    },
    {
      "time": 8,
      "queue": ["P2"],
      "executing": "P2"
    }
  ]
}
```

#### Response Fields

```typescript
interface SchedulingResult {
  algorithm: string
  
  processes: Array<{
    id: string                    // Process identifier
    arrivalTime: number          // Input arrival time
    executionTime: number        // Input execution time
    completionTime: number       // Time when process finishes
    waitingTime: number          // Total time spent waiting
    turnaroundTime: number       // Completion time - arrival time
  }>
  
  metrics: {
    averageWaitTime: number      // Sum of wait times / n
    averageTurnaroundTime: number // Sum of turnaround times / n
    cpuUtilization: number       // (Total exec time) / (Total elapsed time)
    makespan: number             // Time until last process completes
  }
  
  ganttChart: Array<{
    processId: string            // Which process
    startTime: number           // Start time unit
    endTime: number             // End time unit
    type: "execution" | "waiting" // Segment type
  }>
  
  queueTimeline: Array<{
    time: number                // Time unit
    queue: string[]             // Ready processes (queue contents)
    executing: string | null    // Currently executing process
  }>
}
```

#### Response (Error 400 - Validation Error)

```json
{
  "error": "Missing required field: algorithm"
}
```

#### Response (Error 500 - Scheduler Error)

```json
{
  "error": "Scheduler backend not found. Run 'make' to compile the C backend."
}
```

---

## Data Models

### Process

```typescript
interface Process {
  id: string           // Unique identifier (P1, P2, task_a, etc.)
  arrivalTime: number  // When process enters ready queue
  executionTime: number // Total CPU time needed
  priority: number     // Priority level (1-10)
}
```

**Constraints:**
- `id`: 1-16 characters, alphanumeric
- `arrivalTime`: 0-10000
- `executionTime`: 1-1000
- `priority`: 1-10

### AlgorithmConfig

```typescript
interface AlgorithmConfig {
  algorithm: 'fifo' | 'priority' | 'roundrobin' | 'srt' | 'multilevel' | 'multilevel_dynamic'
  quantum?: number                    // Required for RR, multilevel, multilevel_dynamic
  priorityOrder?: 'ascending' | 'descending'  // For priority algorithm
}
```

**Algorithm Selection:**

| Algorithm | Requires Quantum | Requires Priority Order |
|-----------|-----------------|----------------------|
| `fifo` | No | No |
| `priority` | No | Optional (default: descending) |
| `roundrobin` | Yes (1-20) | No |
| `srt` | No | No |
| `multilevel` | Yes (1-20) | No |
| `multilevel_dynamic` | Yes (1-20) | No |

### GanttSegment

```typescript
interface GanttSegment {
  processId: string
  startTime: number      // Inclusive
  endTime: number        // Exclusive
  type: 'execution' | 'waiting'
}
```

**Rendering Notes:**
- Use `startTime` to `endTime` for X-axis positioning
- `execution`: Blue/solid
- `waiting`: Grey/dashed

---

## Error Handling

### HTTP Status Codes

| Code | Meaning | Common Cause |
|------|---------|------------|
| **200** | Success | Request processed successfully |
| **400** | Bad Request | Invalid input format or missing fields |
| **404** | Not Found | Endpoint doesn't exist (future use) |
| **500** | Server Error | Backend crash or missing binary |
| **504** | Gateway Timeout | Simulation exceeded time limit |

### Error Response Format

```json
{
  "error": "Description of what went wrong",
  "details": {
    "field": "algorithm",
    "issue": "Invalid algorithm name"
  }
}
```

### Common Errors

**Missing File:**
```json
{
  "error": "Configuration file not found",
  "details": {
    "file": "config/nonexistent.txt",
    "suggestion": "Use 'sample_config.txt' or check file path"
  }
}
```

**Invalid Process Format:**
```json
{
  "error": "Invalid process format. Expected: NAME ARRIVAL EXECUTION PRIORITY",
  "details": {
    "line": 3,
    "content": "P1 invalid_arrival 5 1"
  }
}
```

**Backend Not Found:**
```json
{
  "error": "Scheduler backend not found. Run 'make' to compile.",
  "details": {
    "binary": "ordonnanceur",
    "expected_path": "/app/ordonnanceur"
  }
}
```

---

## Rate Limiting

### Current Policy

**No rate limiting** – Unlimited requests

### Recommended Future Policy (if public deployment)

```
- 100 requests per minute per IP
- 10 concurrent requests per user
- 30-second timeout for long simulations
```

**Implementation:**

```typescript
// Middleware for rate limiting
import rateLimit from 'express-rate-limit'

const limiter = rateLimit({
  windowMs: 60 * 1000,  // 1 minute
  max: 100,             // 100 requests per windowMs
  message: 'Too many requests, please try again later'
})

app.use('/api/', limiter)
```

---

## Code Examples

### cURL

**Parse Config File:**
```bash
curl -X POST http://localhost:3000/api/parse-config \
  -H "Content-Type: application/json" \
  -d '{"defaultFile":"sample_config.txt"}' \
  | jq .
```

**Upload Config File:**
```bash
curl -X POST http://localhost:3000/api/parse-config \
  -F "file=@config.txt" \
  | jq .
```

**Run Simulation:**
```bash
curl -X POST http://localhost:3000/api/schedule \
  -H "Content-Type: application/json" \
  -d '{
    "processes": [
      {"id":"P1","arrivalTime":0,"executionTime":5,"priority":1},
      {"id":"P2","arrivalTime":2,"executionTime":3,"priority":2}
    ],
    "config": {
      "algorithm":"roundrobin",
      "quantum":2
    }
  }' \
  | jq .
```

### JavaScript/TypeScript

**Using Fetch API:**
```typescript
async function scheduleSimulation(
  processes: Process[],
  config: AlgorithmConfig
): Promise<SchedulingResult> {
  const response = await fetch('/api/schedule', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ processes, config })
  })

  if (!response.ok) {
    throw new Error(`API error: ${response.statusText}`)
  }

  return response.json()
}

// Usage
const result = await scheduleSimulation(
  [
    { id: 'P1', arrivalTime: 0, executionTime: 5, priority: 1 },
    { id: 'P2', arrivalTime: 2, executionTime: 3, priority: 2 }
  ],
  { algorithm: 'roundrobin', quantum: 2 }
)

console.log('Results:', result)
```

**Using Axios:**
```typescript
import axios from 'axios'

const api = axios.create({
  baseURL: 'http://localhost:3000/api',
  timeout: 30000
})

async function runScheduler() {
  try {
    const { data } = await api.post('/schedule', {
      processes: [...],
      config: { algorithm: 'fifo' }
    })
    console.log('Gantt:', data.ganttChart)
    console.log('Metrics:', data.metrics)
  } catch (error) {
    console.error('Scheduler error:', error.response?.data?.error)
  }
}
```

### Python

```python
import requests
import json

BASE_URL = "http://localhost:3000/api"

def run_scheduler(processes, algorithm, quantum=2):
    payload = {
        "processes": processes,
        "config": {
            "algorithm": algorithm,
            "quantum": quantum
        }
    }
    
    response = requests.post(
        f"{BASE_URL}/schedule",
        json=payload,
        timeout=30
    )
    
    if response.status_code == 200:
        return response.json()
    else:
        print(f"Error: {response.json()['error']}")
        return None

# Usage
processes = [
    {"id": "P1", "arrivalTime": 0, "executionTime": 5, "priority": 1},
    {"id": "P2", "arrivalTime": 2, "executionTime": 3, "priority": 2},
]

result = run_scheduler(processes, "roundrobin", quantum=2)
if result:
    print(f"Average Wait Time: {result['metrics']['averageWaitTime']}")
    print(f"Makespan: {result['metrics']['makespan']}")
```

### React Component

```typescript
import { useState } from 'react'

export function SchedulerDemo() {
  const [results, setResults] = useState<SchedulingResult | null>(null)
  const [loading, setLoading] = useState(false)

  const handleSimulate = async () => {
    setLoading(true)
    try {
      const response = await fetch('/api/schedule', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          processes: [
            { id: 'P1', arrivalTime: 0, executionTime: 5, priority: 1 },
            { id: 'P2', arrivalTime: 2, executionTime: 3, priority: 2 }
          ],
          config: { algorithm: 'roundrobin', quantum: 2 }
        })
      })

      if (response.ok) {
        const data = await response.json()
        setResults(data)
      }
    } catch (error) {
      console.error('Failed to run simulation:', error)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div>
      <button onClick={handleSimulate} disabled={loading}>
        {loading ? 'Running...' : 'Run Simulation'}
      </button>
      {results && (
        <div>
          <p>Average Wait Time: {results.metrics.averageWaitTime.toFixed(2)}</p>
          <p>Makespan: {results.metrics.makespan}</p>
        </div>
      )}
    </div>
  )
}
```

---

## Webhooks

### Future Enhancement

For production deployment, consider implementing webhooks:

```typescript
interface WebhookConfig {
  url: string              // Where to POST results
  event: 'simulation.complete' | 'error'
  retryPolicy: {
    maxAttempts: number
    backoffMultiplier: number
  }
}
```

**Example Webhook Payload:**
```json
{
  "event": "simulation.complete",
  "timestamp": "2024-01-15T10:30:00Z",
  "simulation": {
    "id": "sim_12345",
    "algorithm": "roundrobin",
    "processCount": 5,
    "metrics": {
      "averageWaitTime": 3.5,
      "makespan": 20
    }
  }
}
```

---

## Pagination

### Current Status

**Not Implemented** – All data returned in single response

### Future Enhancement (if needed)

```typescript
// For large result sets, could implement:
interface PaginatedResponse {
  data: SchedulingResult[]
  pagination: {
    total: number
    page: number
    pageSize: number
    totalPages: number
  }
}

// Usage:
GET /api/schedule?page=2&pageSize=10
```

---

**Last Updated:** December 2024  
**Version:** 1.0.0  
**API Status:** Stable ✅


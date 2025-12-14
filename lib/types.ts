export interface Process {
  id: string
  arrivalTime: number
  priority: number
  executionTime: number
}

export interface AlgorithmConfig {
  algorithm: "fifo" | "priority" | "roundrobin" | "srt" | "multilevel" | "multilevel_dynamic"
  priorityOrder?: "ascending" | "descending"
  quantum?: number
}

export interface SchedulingResult {
  ganttData: Array<{
    process: string
    start: number
    end: number
    duration: number
  }>
  processStats: Array<{
    id: string
    waitTime: number
    totalTime: number
    arrivalTime: number
    executionTime: number
    finishTime: number
    priority: number
    finalPriority?: number
  }>
  algorithm: string
  averageWait?: number
  makespan?: number
}

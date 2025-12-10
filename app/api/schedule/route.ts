/// <reference types="node" />
import { NextResponse } from "next/server"
import { promises as fs } from "fs"
import os from "os"
import path from "path"
import { spawn, type ChildProcessWithoutNullStreams } from "child_process"
import type { AlgorithmConfig, Process, SchedulingResult } from "@/lib/types"

interface ScheduleRequest {
  processes: Process[]
  config: AlgorithmConfig
}

async function runSchedulerBackend(tmpConfigPath: string, config: AlgorithmConfig): Promise<SchedulingResult> {
  const mapped = { name: config.algorithm, prioMode: config.priorityOrder === "ascending" ? 0 : 1 }
  const binaryName = process.platform === "win32" ? "ordonnanceur.exe" : "ordonnanceur"
  const binaryPath = path.join(process.cwd(), binaryName)

  try {
    await fs.access(binaryPath)
  } catch {
    throw new Error(`Binaire backend introuvable (${binaryPath}). Exécutez 'make' pour le générer.`)
  }

  const args = ["--api", "--config", tmpConfigPath, "--algo", mapped.name]
  if (config.quantum) {
    args.push("--quantum", String(config.quantum))
  }
  if (mapped.prioMode !== undefined) {
    args.push("--prio-order", mapped.prioMode === 0 ? "asc" : "desc")
  }

  const output = await new Promise<string>((resolve, reject) => {
    const child: ChildProcessWithoutNullStreams = spawn(binaryPath, args, { stdio: ["ignore", "pipe", "pipe"] })
    let stdout = ""
    let stderr = ""

    child.stdout.on("data", (data: Uint8Array) => {
      stdout += data.toString()
    })

    child.stderr.on("data", (data: Uint8Array) => {
      stderr += data.toString()
    })

    child.on("error", (err: Error) => {
      reject(err)
    })

    child.on("close", (code: number | null) => {
      if (code === 0) {
        resolve(stdout)
      } else {
        reject(new Error(stderr || `Scheduler exited with code ${code}`))
      }
    })
  })

  try {
    return JSON.parse(output.trim()) as SchedulingResult
  } catch (err) {
    throw new Error(`Invalid backend output: ${(err as Error).message} -> ${output}`)
  }
}

export async function POST(req: Request) {
  let tmpPath = ""
  try {
    const body = (await req.json()) as ScheduleRequest
    if (!body?.processes?.length) {
      return NextResponse.json({ error: "Aucun processus fourni" }, { status: 400 })
    }

    const lines = body.processes
      .map((p) => `${p.id} ${p.arrivalTime} ${p.executionTime} ${p.priority}`)
      .join("\n")
    tmpPath = path.join(os.tmpdir(), `sched_${Date.now()}_${Math.random().toString(16).slice(2)}.txt`)
    await fs.writeFile(tmpPath, lines, "utf8")

    const result = await runSchedulerBackend(tmpPath, body.config)

    return NextResponse.json(result)
  } catch (error) {
    const message = error instanceof Error ? error.message : "Unexpected error"
    return NextResponse.json({ error: message }, { status: 500 })
  } finally {
    if (tmpPath) {
      await fs.unlink(tmpPath).catch(() => {})
    }
  }
}

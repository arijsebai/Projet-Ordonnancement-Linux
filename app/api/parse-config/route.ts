import { NextResponse } from "next/server"
import { promises as fs } from "fs"
import os from "os"
import path from "path"
import { spawn, type ChildProcessWithoutNullStreams } from "child_process"

export async function POST(req: Request) {
  let tmpPath = ""
  try {
    const formData = await req.formData()
    const file = formData.get("file") as File
    if (!file) {
      return NextResponse.json({ error: "Aucun fichier fourni" }, { status: 400 })
    }

    const buffer = await file.arrayBuffer()
    tmpPath = path.join(os.tmpdir(), `config_${Date.now()}_${Math.random().toString(16).slice(2)}.txt`)
    await fs.writeFile(tmpPath, Buffer.from(buffer))

    const binaryName = process.platform === "win32" ? "ordonnanceur.exe" : "ordonnanceur"
    const binaryPath = path.join(process.cwd(), binaryName)

    try {
      await fs.access(binaryPath)
    } catch {
      return NextResponse.json(
        { error: `Binaire backend introuvable (${binaryPath}). Exécutez 'make' pour le générer.` },
        { status: 500 }
      )
    }

    const output = await new Promise<string>((resolve, reject) => {
      const child: ChildProcessWithoutNullStreams = spawn(binaryPath, ["--parse-config", tmpPath], {
        stdio: ["ignore", "pipe", "pipe"],
      })
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
          reject(new Error(stderr || `Parser exited with code ${code}`))
        }
      })
    })

    try {
      return NextResponse.json(JSON.parse(output.trim()))
    } catch (err) {
      return NextResponse.json({ error: `Invalid parser output: ${(err as Error).message}` }, { status: 500 })
    }
  } catch (error) {
    const message = error instanceof Error ? error.message : "Unexpected error"
    return NextResponse.json({ error: message }, { status: 500 })
  } finally {
    if (tmpPath) {
      await fs.unlink(tmpPath).catch(() => {})
    }
  }
}

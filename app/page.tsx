"use client"

import { useState, useEffect } from "react"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { FileGenerationDialog } from "@/components/file-generation-dialog"
import { AlgorithmSelector } from "@/components/algorithm-selector"
import { ResultsDisplay } from "@/components/results-display"
import { FileUp, FilePlus, Play } from "lucide-react"
import type { Process, SchedulingResult, AlgorithmConfig } from "@/lib/types"

export default function Page() {
  const [processes, setProcesses] = useState<Process[]>([])
  const [showFileDialog, setShowFileDialog] = useState(false)
  const [algorithmConfig, setAlgorithmConfig] = useState<AlgorithmConfig>({
    algorithm: "fifo",
  })
  const [results, setResults] = useState<SchedulingResult | null>(null)
  const [isRunning, setIsRunning] = useState(false)
  const [showProcessTable, setShowProcessTable] = useState(false)
  const [loadingDefault, setLoadingDefault] = useState(true)
  const [currentFileName, setCurrentFileName] = useState<string>("sample_config.txt")

  
  useEffect(() => {
    const loadDefaultConfig = async () => {
      try {
        const response = await fetch("/api/parse-config", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ defaultFile: "sample_config.txt" }),
        })

        if (response.ok) {
          const processes = await response.json()
          setProcesses(processes)
          setCurrentFileName("sample_config.txt")
        }
      } catch (err) {
        console.error("Erreur lors du chargement du fichier par défaut:", err)
      } finally {
        setLoadingDefault(false)
      }
    }

    loadDefaultConfig()
  }, [])

  const handleGenerateFile = (generatedProcesses: Process[]) => {
    setProcesses(generatedProcesses)
    setCurrentFileName("Fichier généré")
    setShowFileDialog(false)
  }

  const handleChooseFile = () => {
    const input = document.createElement("input")
    input.type = "file"
    input.accept = ".txt"
    input.onchange = async (e) => {
      const file = (e.target as HTMLInputElement).files?.[0]
      if (file) {
        const formData = new FormData()
        formData.append("file", file)

        try {
          const response = await fetch("/api/parse-config", {
            method: "POST",
            body: formData,
          })

          if (!response.ok) {
            const message = await response.json().catch(() => ({}))
            throw new Error(message?.error || "Erreur lors du parsing")
          }

          const processes = await response.json()
          setProcesses(processes)
          setCurrentFileName(file.name)
        } catch (err) {
          alert(`Erreur : ${(err as Error).message}`)
        }
      }
    }
    input.click()
  }

  const handleLaunchScheduling = async () => {
    if (processes.length === 0) {
      alert("Veuillez d'abord créer ou choisir un fichier de processus")
      return
    }

    setIsRunning(true)
    try {
      const response = await fetch("/api/schedule", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ processes, config: algorithmConfig }),
      })

      if (!response.ok) {
        const message = await response.json().catch(() => ({}))
        throw new Error(message?.error || "Une erreur est survenue")
      }

      const payload = (await response.json()) as SchedulingResult
      setResults(payload)
    } catch (err) {
      console.error(err)
      alert(`Erreur lors de l'exécution de l'ordonnanceur: ${(err as Error).message}`)
    } finally {
      setIsRunning(false)
    }
  }

  return (
    <div className="min-h-screen p-8 bg-background">
      <div className="max-w-7xl mx-auto space-y-8">
        {}
        <div className="text-center space-y-4">
          <h1 className="text-5xl font-bold text-white drop-shadow-md">Ordonnancement des Processus</h1>
          <p className="text-white/80 text-lg">
            Simulateur d'algorithmes d'ordonnancement avec visualisation en temps réel
          </p>
        </div>

        {!results ? (
          <div className="grid gap-8 lg:grid-cols-2">
            {}
            <Card className="bg-white border-white/20 shadow-xl">
              <CardHeader>
                <CardTitle className="text-2xl flex items-center gap-2 text-black">
                  <FilePlus className="h-6 w-6 text-primary" />
                  Gestion des Fichiers
                </CardTitle>
                <CardDescription className="text-gray-600">
                  Fichier par défaut chargé. Vous pouvez en choisir un autre ou en générer un nouveau
                </CardDescription>
              </CardHeader>
              <CardContent className="space-y-4">
                <div className="p-4 bg-blue-100 border border-blue-300 rounded-lg">
                  <p className="text-blue-700 font-medium text-center">
                    ℹ️ Fichier actuel : <span className="font-bold">{currentFileName}</span>
                  </p>
                </div>

                <Button
                  onClick={() => setShowFileDialog(true)}
                  variant="outline"
                  className="w-full h-20 text-lg border-2 border-[#500010] text-black bg-transparent hover:bg-[#500010] hover:text-white transition-colors duration-300"
                  size="lg"
                >
                  <FilePlus className="mr-2 h-5 w-5" />
                  Générer un Fichier
                </Button>
                <Button
                  onClick={handleChooseFile}
                  variant="outline"
                  className="w-full h-20 text-lg border-2 border-[#500010] text-black bg-transparent hover:bg-[#500010] hover:text-white transition-colors duration-300"
                  size="lg"
                >
                  <FileUp className="mr-2 h-5 w-5" />
                  Choisir un Fichier
                </Button>

                {!loadingDefault && processes.length > 0 && (
                  <div className="mt-4 p-4 bg-green-100 border border-green-300 rounded-lg">
                    <p className="text-green-700 font-medium text-center">✓ {processes.length} processus chargés</p>
                    <Button
                      onClick={() => setShowProcessTable(!showProcessTable)}
                      variant="link"
                      className="w-full text-green-700 underline mt-2"
                    >
                      {showProcessTable ? "Masquer les détails" : "Afficher les détails"}
                    </Button>
                    {showProcessTable && (
                      <div className="mt-2 overflow-x-auto text-xs">
                        <table className="w-full border-collapse border border-green-300">
                          <thead className="bg-green-200">
                            <tr>
                              <th className="border border-green-300 px-2 py-1">ID</th>
                              <th className="border border-green-300 px-2 py-1">Arrivée</th>
                              <th className="border border-green-300 px-2 py-1">Exécution</th>
                              <th className="border border-green-300 px-2 py-1">Priorité</th>
                            </tr>
                          </thead>
                          <tbody>
                            {processes.map((p) => (
                              <tr key={p.id}>
                                <td className="border border-green-300 px-2 py-1">{p.id}</td>
                                <td className="border border-green-300 px-2 py-1">{p.arrivalTime}</td>
                                <td className="border border-green-300 px-2 py-1">{p.executionTime}</td>
                                <td className="border border-green-300 px-2 py-1">{p.priority}</td>
                              </tr>
                            ))}
                          </tbody>
                        </table>
                      </div>
                    )}
                  </div>
                )}
              </CardContent>
            </Card>

            {}
            <Card className="bg-white border-white/20 shadow-xl">
              <CardHeader>
                <CardTitle className="text-2xl text-black">Configuration de l'Algorithme</CardTitle>
                <CardDescription className="text-gray-600">
                  Sélectionnez l'algorithme d'ordonnancement et ses paramètres
                </CardDescription>
              </CardHeader>
              <CardContent>
                <AlgorithmSelector config={algorithmConfig} onChange={setAlgorithmConfig} />
              </CardContent>
            </Card>
          </div>
        ) : null}

        {}
        {!results && (
          <div className="flex justify-center">
            <Button
              onClick={handleLaunchScheduling}
              size="lg"
              className="h-16 px-12 text-xl bg-gradient-to-r from-[#500010] to-[#800020] hover:from-[#800020] hover:to-[#a00030] text-white shadow-lg transition-all duration-300 transform hover:scale-105"
              disabled={isRunning}
            >
              <Play className="mr-2 h-6 w-6" />
              {isRunning ? "Calcul en cours..." : "Lancer l'Ordonnancement"}
            </Button>
          </div>
        )}

        {}
        {results && <ResultsDisplay results={results} onReset={() => setResults(null)} />}
      </div>

      {}
      <FileGenerationDialog open={showFileDialog} onOpenChange={setShowFileDialog} onGenerate={handleGenerateFile} />
    </div>
  )
}


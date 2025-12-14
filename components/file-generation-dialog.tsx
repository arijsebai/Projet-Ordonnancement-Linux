"use client"

import { useState } from "react"
import { Dialog, DialogContent, DialogDescription, DialogHeader, DialogTitle } from "@/components/ui/dialog"
import { Label } from "@/components/ui/label"
import { Input } from "@/components/ui/input"
import { Button } from "@/components/ui/button"
import type { Process } from "@/lib/types"

interface FileGenerationDialogProps {
  open: boolean
  onOpenChange: (open: boolean) => void
  onGenerate: (processes: Process[]) => void
}

export function FileGenerationDialog({ open, onOpenChange, onGenerate }: FileGenerationDialogProps) {
  const [numProcesses, setNumProcesses] = useState("5")
  const [maxArrivalTime, setMaxArrivalTime] = useState("10")
  const [minPriority, setMinPriority] = useState("1")
  const [maxPriority, setMaxPriority] = useState("10")
  const [maxExecutionTime, setMaxExecutionTime] = useState("20")

  const handleGenerate = () => {
    const num = Number.parseInt(numProcesses)
    const maxArrival = Number.parseInt(maxArrivalTime)
    const minPrio = Number.parseInt(minPriority)
    const maxPrio = Number.parseInt(maxPriority)
    const maxExec = Number.parseInt(maxExecutionTime)

    const processes: Process[] = []

    for (let i = 1; i <= num; i++) {
      processes.push({
        id: `P${i}`,
        arrivalTime: Math.floor(Math.random() * (maxArrival + 1)),
        priority: Math.floor(Math.random() * (maxPrio - minPrio + 1)) + minPrio,
        executionTime: Math.floor(Math.random() * maxExec) + 1,
      })
    }

    onGenerate(processes)
  }

  return (
    <Dialog open={open} onOpenChange={onOpenChange}>
      <DialogContent className="sm:max-w-[500px] bg-white text-black border-border">
        <DialogHeader>
          <DialogTitle className="text-2xl text-black">Génération de Fichier</DialogTitle>
          <DialogDescription className="text-gray-600">
            Définissez les paramètres pour générer automatiquement les processus
          </DialogDescription>
        </DialogHeader>

        <div className="space-y-6 py-4">
          <div className="space-y-2">
            <Label htmlFor="numProcesses" className="text-black">
              Nombre de processus
            </Label>
            <Input
              id="numProcesses"
              type="number"
              min="1"
              value={numProcesses}
              onChange={(e) => setNumProcesses(e.target.value)}
              className="bg-white border-gray-300 text-black focus:border-primary"
            />
          </div>

          <div className="space-y-2">
            <Label htmlFor="maxArrivalTime" className="text-black">
              Temps d'arrivée maximum
            </Label>
            <Input
              id="maxArrivalTime"
              type="number"
              min="0"
              value={maxArrivalTime}
              onChange={(e) => setMaxArrivalTime(e.target.value)}
              className="bg-white border-gray-300 text-black focus:border-primary"
            />
          </div>

          <div className="grid grid-cols-2 gap-4">
            <div className="space-y-2">
              <Label htmlFor="minPriority" className="text-black">
                Priorité minimale
              </Label>
              <Input
                id="minPriority"
                type="number"
                min="1"
                value={minPriority}
                onChange={(e) => setMinPriority(e.target.value)}
                className="bg-white border-gray-300 text-black focus:border-primary"
              />
            </div>

            <div className="space-y-2">
              <Label htmlFor="maxPriority" className="text-black">
                Priorité maximale
              </Label>
              <Input
                id="maxPriority"
                type="number"
                min="1"
                value={maxPriority}
                onChange={(e) => setMaxPriority(e.target.value)}
                className="bg-white border-gray-300 text-black focus:border-primary"
              />
            </div>
          </div>

          <div className="space-y-2">
            <Label htmlFor="maxExecutionTime" className="text-black">
              Temps d'exécution maximum
            </Label>
            <Input
              id="maxExecutionTime"
              type="number"
              min="1"
              value={maxExecutionTime}
              onChange={(e) => setMaxExecutionTime(e.target.value)}
              className="bg-white border-gray-300 text-black focus:border-primary"
            />
          </div>
        </div>

        <Button
          onClick={handleGenerate}
          className="w-full bg-[#500010] hover:bg-[#800020] text-white transition-colors duration-300"
          size="lg"
        >
          Générer
        </Button>
      </DialogContent>
    </Dialog>
  )
}

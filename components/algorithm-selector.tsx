"use client"

import { Label } from "@/components/ui/label"
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select"
import { RadioGroup, RadioGroupItem } from "@/components/ui/radio-group"
import { Input } from "@/components/ui/input"
import type { AlgorithmConfig } from "@/lib/types"

interface AlgorithmSelectorProps {
  config: AlgorithmConfig
  onChange: (config: AlgorithmConfig) => void
}

export function AlgorithmSelector({ config, onChange }: AlgorithmSelectorProps) {
  const showPriorityOrder = config.algorithm === "priority"
  const showQuantum =
    config.algorithm === "roundrobin" || config.algorithm === "multilevel" || config.algorithm === "multilevel_dynamic"

  return (
    <div className="space-y-6">
      <div className="space-y-3">
        <Label htmlFor="algorithm" className="text-base text-foreground">
          Algorithme d'ordonnancement
        </Label>
        <Select value={config.algorithm} onValueChange={(value) => onChange({ ...config, algorithm: value as any })}>
          <SelectTrigger id="algorithm" className="bg-background border-input text-foreground">
            <SelectValue placeholder="Sélectionnez un algorithme" />
          </SelectTrigger>
          <SelectContent className="bg-popover border-border">
            <SelectItem value="fifo" className="text-popover-foreground">
              FIFO (First In First Out)
            </SelectItem>
            <SelectItem value="priority" className="text-popover-foreground">
              Priorité Préemptive
            </SelectItem>
            <SelectItem value="roundrobin" className="text-popover-foreground">
              Round Robin
            </SelectItem>
            <SelectItem value="multilevel" className="text-popover-foreground">
              Multilevel
            </SelectItem>
            <SelectItem value="multilevel_dynamic" className="text-popover-foreground">
              Multilevel - Priorité Dynamique
            </SelectItem>
            <SelectItem value="srt" className="text-popover-foreground">
              SRT (Shortest Remaining Time)
            </SelectItem>
          </SelectContent>
        </Select>
      </div>

      {showPriorityOrder && (
        <div className="space-y-3 p-4 bg-muted/30 rounded-lg border border-border/50">
          <Label className="text-base text-black">Ordre de priorité</Label>
          <RadioGroup
            value={config.priorityOrder || "ascending"}
            onValueChange={(value) => onChange({ ...config, priorityOrder: value as "ascending" | "descending" })}
            className="space-y-2"
          >
            <div className="flex items-center space-x-3">
              <RadioGroupItem value="ascending" id="ascending" className="border-primary text-primary" />
              <Label htmlFor="ascending" className="font-normal cursor-pointer text-black">
                Ascendant (priorité basse en premier)
              </Label>
            </div>
            <div className="flex items-center space-x-3">
              <RadioGroupItem value="descending" id="descending" className="border-primary text-primary" />
              <Label htmlFor="descending" className="font-normal cursor-pointer text-black">
                Descendant (priorité haute en premier)
              </Label>
            </div>
          </RadioGroup>
        </div>
      )}

      {showQuantum && (
        <div className="space-y-3 p-4 bg-muted/30 rounded-lg border border-border/50">
          <Label htmlFor="quantum" className="text-base text-black">
            Quantum de temps
          </Label>
          <Input
            id="quantum"
            type="number"
            min="1"
            value={config.quantum || 2}
            onChange={(e) => onChange({ ...config, quantum: Number.parseInt(e.target.value) })}
            className="bg-background border-input text-foreground"
            placeholder="Entrez le quantum"
          />
        </div>
      )}

      {/* multilevel uses quantum; no extra level parameter required for current backend */}
    </div>
  )
}

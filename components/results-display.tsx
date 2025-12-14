"use client"

import { useEffect, useRef, useState } from "react"
import { Bar, BarChart, CartesianGrid, Cell, Legend, Pie, PieChart, ResponsiveContainer, XAxis, YAxis } from "recharts"
import { Pause, Play, RotateCcw, SkipBack, StepBack, StepForward } from "lucide-react"
import type { SchedulingResult } from "@/lib/types"
import { Button } from "@/components/ui/button"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"
import { ChartContainer, ChartTooltip, ChartTooltipContent } from "@/components/ui/chart"

interface ResultsDisplayProps {
  results: SchedulingResult
  onReset: () => void
}

const PALETTE = [
  "#EF4444",
  "#10B981",
  "#3B82F6",
  "#F59E0B",
  "#8B5CF6",
  "#EC4899",
  "#14B8A6",
  "#F97316",
  "#22C55E",
  "#6366F1",
  "#0EA5E9",
  "#84CC16",
  "#D946EF",
  "#E11D48",
  "#A855F7",
  "#06B6D4",
  "#F43F5E",
  "#FACC15",
  "#34D399",
  "#60A5FA",
]

const WAIT_TIME_COLOR = "#00008B"
const TOTAL_TIME_COLOR = "#006400"

export function ResultsDisplay({ results, onReset }: ResultsDisplayProps) {
  const boundaries = Array.from(
    new Set([
      0,
      ...(results.ganttData ?? []).flatMap((g) => [g.start, g.end]),
      results.makespan ?? 0,
    ])
  )
    .filter((v) => v >= 0)
    .sort((a, b) => a - b)

  const axisEnd = boundaries[boundaries.length - 1] || 1
  const timeTicks = boundaries.length > 0 ? boundaries : [0, axisEnd]
  const minorTicks: number[] = []
  const maxTime = axisEnd

  const [currentTime, setCurrentTime] = useState(maxTime)
  const [isPlaying, setIsPlaying] = useState(false)
  const intervalRef = useRef<NodeJS.Timeout | null>(null)

  const [queueTime, setQueueTime] = useState(maxTime)
  const [isQueuePlaying, setIsQueuePlaying] = useState(false)
  const queueIntervalRef = useRef<NodeJS.Timeout | null>(null)

  useEffect(() => {
    if (isPlaying) {
      intervalRef.current = setInterval(() => {
        setCurrentTime((prev) => {
          if (prev >= maxTime) {
            setIsPlaying(false)
            return maxTime
          }
          return prev + 1
        })
      }, 1000)
    } else if (intervalRef.current) {
      clearInterval(intervalRef.current)
    }

    return () => {
      if (intervalRef.current) clearInterval(intervalRef.current)
    }
  }, [isPlaying, maxTime])

  useEffect(() => {
    if (isQueuePlaying) {
      queueIntervalRef.current = setInterval(() => {
        setQueueTime((prev) => {
          if (prev >= maxTime) {
            setIsQueuePlaying(false)
            return maxTime
          }
          return prev + 1
        })
      }, 1000)
    } else if (queueIntervalRef.current) {
      clearInterval(queueIntervalRef.current)
    }

    return () => {
      if (queueIntervalRef.current) clearInterval(queueIntervalRef.current)
    }
  }, [isQueuePlaying, maxTime])

  const togglePlay = () => {
    if (currentTime >= maxTime) setCurrentTime(0)
    setIsPlaying((p) => !p)
  }

  const toggleQueuePlay = () => {
    if (queueTime >= maxTime) setQueueTime(0)
    setIsQueuePlaying((p) => !p)
  }

  const handleNextStep = () => setCurrentTime((prev) => Math.min(prev + 1, maxTime))
  const handlePrevStep = () => {
    setCurrentTime((prev) => Math.max(prev - 1, 0))
    setIsPlaying(false)
  }
  const handleResetTime = () => {
    setCurrentTime(0)
    setIsPlaying(false)
  }

  const handleQueueNextStep = () => setQueueTime((prev) => Math.min(prev + 1, maxTime))
  const handleQueuePrevStep = () => {
    setQueueTime((prev) => Math.max(prev - 1, 0))
    setIsQueuePlaying(false)
  }
  const handleQueueReset = () => {
    setQueueTime(0)
    setIsQueuePlaying(false)
  }

  const axisSafe = axisEnd || 1
  const currentLimit = Math.min(Math.max(0, currentTime), axisSafe)
  const queueCurrentLimit = Math.min(Math.max(0, queueTime), axisSafe)

  const waitingSegments = new Map<string, Array<{ start: number; end: number }>>()
  const arrivalMap = new Map<string, number>()
  results.processStats.forEach((stat) => arrivalMap.set(stat.id, stat.arrivalTime))
  results.processStats.forEach((stat) => {
    const segs = results.ganttData
      .filter((g) => g.process === stat.id)
      .sort((a, b) => a.start - b.start)
    const waits: Array<{ start: number; end: number }> = []
    let cursor = stat.arrivalTime
    for (const s of segs) {
      if (s.start > cursor) waits.push({ start: cursor, end: s.start })
      cursor = s.end
    }
    waitingSegments.set(stat.id, waits)
  })

  const uniqueProcesses = Array.from(
    new Set([...results.ganttData.map((g) => g.process), ...results.processStats.map((s) => s.id)])
  ).sort()

  const colorMap = new Map<string, string>()
  uniqueProcesses.forEach((pid, idx) => {
    const base = idx < PALETTE.length ? PALETTE[idx] : `hsl(${(idx * 137) % 360}deg 70% 45%)`
    colorMap.set(pid, base)
  })
  const colorFor = (pid: string) => colorMap.get(pid) || PALETTE[0]

  const cpuSegments = [...results.ganttData].sort((a, b) => a.start - b.start)

  const timeHorizon = Math.max(1, Math.ceil(axisSafe))
  const queueTimelineRaw: Array<{ time: number; waiting: string[] }> = []
  for (let t = 0; t < timeHorizon; t++) {
    const running = cpuSegments.find((seg) => t >= seg.start && t < seg.end)
    const waiting = results.processStats
      .filter((stat) => stat.arrivalTime <= t && stat.finishTime > t && (!running || stat.id !== running.process))
      .map((stat) => stat.id)
      .sort((a, b) => {
        const aArr = arrivalMap.get(a) ?? 0
        const bArr = arrivalMap.get(b) ?? 0
        if (aArr !== bArr) return aArr - bArr
        return a.localeCompare(b)
      })
    queueTimelineRaw.push({ time: t, waiting })
  }

  const sameQueue = (a: string[], b: string[]) => a.length === b.length && a.every((v, idx) => v === b[idx])
  const queueTimeline: Array<{ start: number; end: number; waiting: string[] }> = []
  for (const entry of queueTimelineRaw) {
    const last = queueTimeline[queueTimeline.length - 1]
    if (last && sameQueue(last.waiting, entry.waiting)) {
      last.end = entry.time + 1
    } else {
      queueTimeline.push({ start: entry.time, end: entry.time + 1, waiting: entry.waiting })
    }
  }

  const queuePerSecond = queueTimelineRaw

  const pieData = results.processStats.map((stat) => ({
    name: stat.id,
    value: stat.executionTime,
    fill: colorFor(stat.id),
  }))

  const barData = results.processStats.map((stat) => ({
    name: stat.id,
    "Temps d'attente": stat.waitTime,
    "Temps de rotation": stat.totalTime,
  }))

  const averageWaitTime =
    results.processStats.reduce((acc, curr) => acc + curr.waitTime, 0) / results.processStats.length
  const averageTurnaroundTime =
    results.processStats.reduce((acc, curr) => acc + curr.totalTime, 0) / results.processStats.length
  const cpuTotalTime = results.processStats.reduce((acc, curr) => acc + curr.executionTime, 0)

  return (
    <div className="space-y-8">
      <div className="flex items-center justify-between">
        <div>
          <h2 className="text-3xl font-bold text-white">Résultats de l'Ordonnancement</h2>
          <p className="text-white/80 mt-1">
            Algorithme: <span className="font-semibold text-white">{results.algorithm.toUpperCase()}</span>
          </p>
        </div>

        <div className="flex items-center gap-2">
          <Button
            onClick={onReset}
            variant="outline"
            className="border-white/50 text-white bg-transparent hover:bg-white hover:text-[#500010] transition-colors duration-300"
          >
            <RotateCcw className="mr-2 h-4 w-4" />
            Nouvelle Simulation
          </Button>
        </div>
      </div>

      {}
      <Card className="bg-white/95 backdrop-blur-sm border-white/20 shadow-xl w-full">
        <CardHeader>
          <CardTitle className="text-black">Diagramme de Gantt</CardTitle>
          <CardDescription className="text-gray-600">
            Occupation CPU & File d'attente
          </CardDescription>
        </CardHeader>
        <CardContent>
          <div className="flex items-center gap-2 mb-4">
            <Button
              onClick={handleQueueReset}
              variant="outline"
              className="border-[#500010] text-[#500010] bg-transparent hover:bg-[#500010] hover:text-white"
              title="Revenir au début"
            >
              <SkipBack className="h-4 w-4" />
            </Button>
            <Button
              onClick={toggleQueuePlay}
              variant="outline"
              className="border-[#500010] text-[#500010] bg-transparent hover:bg-[#500010] hover:text-white"
            >
              {isQueuePlaying ? <Pause className="mr-2 h-4 w-4" /> : <Play className="mr-2 h-4 w-4" />}
              {isQueuePlaying ? "Pause" : "Lecture"}
            </Button>
            <Button
              onClick={handleQueuePrevStep}
              variant="outline"
              className="border-[#500010] text-[#500010] bg-transparent hover:bg-[#500010] hover:text-white"
              disabled={isQueuePlaying || queueTime <= 0}
            >
              <StepBack className="mr-2 h-4 w-4" />
              -1s
            </Button>
            <Button
              onClick={handleQueueNextStep}
              variant="outline"
              className="border-[#500010] text-[#500010] bg-transparent hover:bg-[#500010] hover:text-white"
              disabled={isQueuePlaying || queueTime >= maxTime}
            >
              <StepForward className="mr-2 h-4 w-4" />
              +1s
            </Button>
            <div className="flex items-center px-4 bg-[#500010] text-white rounded-md border border-[#500010]/60 min-w-[100px] justify-center font-mono">
              T = {queueTime}s
            </div>
          </div>
          <div className="relative mt-4 overflow-x-auto">
            <div className="min-w-[600px] space-y-4">
              {}
              <div className="flex ml-16 border-b border-gray-200 pb-2 pr-10">
                {timeTicks.map((t, idx) => {
                  const isLast = idx === timeTicks.length - 1
                  return (
                    <div
                      key={`cpu-axis-${idx}`}
                      className="text-[10px] text-gray-500 text-right border-r border-gray-100 h-2 relative"
                      style={{
                        flexBasis: `${(t - (timeTicks[idx - 1] ?? 0)) / axisEnd * 100}%`,
                        flexGrow: 1,
                        flexShrink: 0,
                      }}
                    >
                      <span
                        className={`absolute -bottom-6 left-full ${isLast ? "translate-x-0" : "-translate-x-1/2"} whitespace-nowrap`}
                      >
                        {t}
                      </span>
                    </div>
                  )
                })}
              </div>

              {}
              <div className="relative">
                <div className="absolute inset-0 flex ml-16 pointer-events-none">
                  {timeTicks.map((t, idx) => (
                    <div
                      key={`cpu-grid-${idx}`}
                      className="border-r border-gray-200 h-full"
                      style={{
                        flexBasis: `${(t - (timeTicks[idx - 1] ?? 0)) / axisEnd * 100}%`,
                        flexGrow: 1,
                        flexShrink: 0,
                      }}
                    />
                  ))}
                </div>

                <div className="relative flex items-center h-12 mb-4">
                  <div className="w-16 font-bold text-gray-700 flex-shrink-0">CPU</div>
                  <div className="flex-grow h-8 bg-gray-50 rounded-r-lg relative overflow-hidden ml-1">
                    {cpuSegments.map((seg, idx) => {
                      const visibleEnd = Math.min(seg.end, queueCurrentLimit)
                      const widthPercent = ((visibleEnd - seg.start) / axisSafe) * 100
                      const leftPercent = (seg.start / axisSafe) * 100
                      if (widthPercent <= 0) return null
                      return (
                        <div
                          key={`cpu-${idx}`}
                          className="absolute top-1 bottom-1 rounded shadow-md flex items-center justify-center text-xs text-white font-medium"
                          style={{
                            left: `${leftPercent}%`,
                            width: `${widthPercent}%`,
                            backgroundColor: colorFor(seg.process),
                          }}
                          title={`${seg.process}: ${seg.start}s → ${seg.end}s (durée ${seg.duration}s)`}
                        >
                          <span className="px-1 overflow-hidden text-ellipsis">{seg.process}</span>
                        </div>
                      )
                    })}
                  </div>
                </div>
              </div>

              {}
              <div>
                <div className="flex items-center gap-2 mb-2 ml-16">
                  <div className="w-3 h-3 rounded-sm bg-gray-400" />
                  <span className="text-sm text-gray-700">File d'attente (processus prêts mais non en CPU)</span>
                </div>

                <div className="relative flex items-center h-20">
                  <div className="w-16 font-bold text-gray-700 flex-shrink-0">Queue</div>
                  <div className="flex-grow h-full bg-gray-50 rounded-r-lg relative overflow-hidden ml-1">
                    {queuePerSecond
                      .filter((seg) => seg.time < queueCurrentLimit)
                      .map((seg, idx) => {
                        const widthPercent = (1 / axisSafe) * 100
                        const leftPercent = (seg.time / axisSafe) * 100
                        const hasWaiting = seg.waiting.length > 0
                        return (
                          <div
                            key={`queue-second-${idx}`}
                            className="absolute top-1 bottom-1 rounded border border-gray-200 bg-white/80 flex flex-col gap-1 items-center justify-start pt-1"
                            style={{ left: `${leftPercent}%`, width: `${widthPercent}%` }}
                            title={hasWaiting ? `T=${seg.time} : ${seg.waiting.join(", ")}` : `T=${seg.time} : file vide`}
                          >
                            {hasWaiting ? (
                              seg.waiting.map((pid) => (
                                <span
                                  key={`${pid}-${idx}`}
                                  className="text-[9px] font-semibold px-1 py-[1px] rounded"
                                  style={{ backgroundColor: colorFor(pid), color: "#fff" }}
                                >
                                  {pid}
                                </span>
                              ))
                            ) : (
                              <span className="text-[9px] text-gray-500">∅</span>
                            )}
                          </div>
                        )
                      })}
                  </div>
                </div>
              </div>
            </div>
          </div>
        </CardContent>
      </Card>

      {}
      <Card className="bg-white/95 backdrop-blur-sm border-white/20 shadow-xl">
        <CardHeader>
          <CardTitle className="text-black">Diagramme de l'état des processus</CardTitle>
          <CardDescription className="text-gray-600">
            Visualisation temporelle de l'exécution des processus
          </CardDescription>
        </CardHeader>
        <CardContent>
          <div className="flex flex-wrap items-center gap-2 mb-4">
            <Button
              onClick={handleResetTime}
              variant="outline"
              className="border-[#500010] text-[#500010] bg-transparent hover:bg-[#500010] hover:text-white"
              title="Revenir au début"
            >
              <SkipBack className="h-4 w-4" />
            </Button>
            <Button
              onClick={togglePlay}
              variant="outline"
              className="border-[#500010] text-[#500010] bg-transparent hover:bg-[#500010] hover:text-white"
            >
              {isPlaying ? <Pause className="mr-2 h-4 w-4" /> : <Play className="mr-2 h-4 w-4" />}
              {isPlaying ? "Pause" : "Lecture"}
            </Button>
            <Button
              onClick={handlePrevStep}
              variant="outline"
              className="border-[#500010] text-[#500010] bg-transparent hover:bg-[#500010] hover:text-white"
              disabled={isPlaying || currentTime <= 0}
            >
              <StepBack className="mr-2 h-4 w-4" />
              Précédent (-1s)
            </Button>
            <Button
              onClick={handleNextStep}
              variant="outline"
              className="border-[#500010] text-[#500010] bg-transparent hover:bg-[#500010] hover:text-white"
              disabled={isPlaying || currentTime >= maxTime}
            >
              <StepForward className="mr-2 h-4 w-4" />
              Suivant (+1s)
            </Button>
            <div className="flex items-center px-4 bg-[#500010] text-white rounded-md border border-[#500010]/60 min-w-[100px] justify-center font-mono">
              T = {currentTime}s
            </div>
          </div>
          <div className="relative mt-4 overflow-x-auto">
            <div className="min-w-[600px] space-y-4">
              {}
              <div className="flex ml-20 border-b border-gray-200 pb-2 pr-10">
                {timeTicks.map((t, idx) => {
                  const isLast = idx === timeTicks.length - 1
                  return (
                    <div
                      key={idx}
                      className="text-xs text-gray-500 text-right border-r border-gray-100 h-2 relative"
                      style={{
                        flexBasis: `${(t - (timeTicks[idx - 1] ?? 0)) / axisEnd * 100}%`,
                        flexGrow: 1,
                        flexShrink: 0,
                      }}
                    >
                      <span
                        className={`absolute -bottom-6 left-full ${isLast ? "translate-x-0" : "-translate-x-1/2"} whitespace-nowrap`}
                      >
                        {t}
                      </span>
                    </div>
                  )
                })}
              </div>

              {}
              <div className="relative">
                {}
                <div className="absolute inset-0 flex ml-20 pointer-events-none">
                  {timeTicks.map((t, idx) => (
                    <div
                      key={`major-${idx}`}
                      className="border-r border-gray-200 h-full"
                      style={{
                        flexBasis: `${(t - (timeTicks[idx - 1] ?? 0)) / axisEnd * 100}%`,
                        flexGrow: 1,
                        flexShrink: 0,
                      }}
                    />
                  ))}

                  {minorTicks.map((t, idx) => (
                    <div
                      key={`minor-${idx}`}
                      className="border-r border-dashed border-gray-100 h-full"
                      style={{ left: `${(t / axisEnd) * 100}%` }}
                    />
                  ))}

                  <div
                    className="absolute top-0 bottom-0 border-l-2 border-red-500 z-20 transition-all duration-300 ease-linear"
                    style={{ left: `${(currentTime / axisEnd) * 100}%` }}
                  />
                </div>

                {uniqueProcesses.map((processId, index) => (
                  <div key={processId} className="relative flex items-center h-12 mb-2 group">
                    {}
                    <div className="w-20 font-bold text-gray-700 flex-shrink-0">{processId}</div>

                    {}
                    <div className="flex-grow h-8 bg-gray-50 rounded-r-lg relative overflow-hidden ml-1">
                      {}
                      {waitingSegments.get(processId)?.map((w, idx) => {
                        const leftPercent = (w.start / axisEnd) * 100
                        const widthPercent = ((w.end - w.start) / axisEnd) * 100
                        if (widthPercent <= 0) return null
                        return (
                          <div
                            key={`wait-${idx}`}
                            className="absolute top-1 bottom-1 rounded z-0 flex items-center justify-center text-[10px] font-semibold"
                            style={{
                              left: `${leftPercent}%`,
                              width: `${widthPercent}%`,
                              backgroundColor: colorFor(processId),
                              opacity: 0.35,
                              color: "#0b0f19",
                            }}
                            title={`En attente: ${w.start} → ${w.end}`}
                          >
                            A
                          </div>
                        )
                      })}

                      {results.ganttData
                        .filter((item) => item.process === processId)
                        .map((item, idx) => {
                          if (item.start >= currentTime) return null 

                          const isFinished = item.end <= currentTime
                          const visibleDuration = isFinished ? item.duration : currentTime - item.start
                          const widthPercent = (visibleDuration / axisEnd) * 100
                          const leftPercent = (item.start / axisEnd) * 100

                          return (
                            <div
                              key={idx}
                              className="absolute top-1 bottom-1 rounded shadow-md flex items-center justify-center text-xs text-white font-medium hover:brightness-110 transition-all cursor-pointer z-10"
                              style={{
                                left: `${leftPercent}%`,
                                width: `${Math.max(widthPercent, 0)}%`, 
                                backgroundColor: colorFor(processId),
                              }}
                              title={`${item.process}: ${item.start}s - ${item.end}s (Durée: ${item.duration}s)`}
                            >
                              <span className="hidden sm:inline overflow-hidden text-ellipsis px-1">
                                {isFinished ? item.duration : visibleDuration}
                              </span>
                            </div>
                          )
                        })}
                    </div>
                  </div>
                ))}
              </div>
            </div>
          </div>
          <div className="mt-3 text-xs text-gray-700 flex items-center gap-2">
            <span className="w-6 h-3 rounded relative overflow-hidden">
              <span className="absolute inset-0 rounded bg-gray-500 opacity-35" />
              <span className="relative z-10 text-[10px] leading-3 flex items-center justify-center text-gray-900 font-semibold">A</span>
            </span>
            <span>A : attente</span>
          </div>
        </CardContent>
      </Card>

      <div className="space-y-8">
        {}
        <Card className="bg-white/95 backdrop-blur-sm border-white/20 shadow-xl w-full">
          <CardHeader>
            <CardTitle className="text-black">Tableau Détaillé des Processus</CardTitle>
            <CardDescription className="text-gray-600">
              Détails complets de l'exécution pour chaque processus
            </CardDescription>
          </CardHeader>
          <CardContent>
            <div className="overflow-x-auto">
              <table className="w-full text-sm text-left text-gray-700">
                <thead className="text-xs text-white uppercase bg-[#500010]">
                  <tr>
                    <th scope="col" className="px-6 py-3 rounded-tl-lg">
                      Nom du Processus
                    </th>
                    <th scope="col" className="px-6 py-3">
                      Temps d'Arrivée
                    </th>
                    <th scope="col" className="px-6 py-3">
                      Temps d'Exécution
                    </th>
                    <th scope="col" className="px-6 py-3">
                      Priorité Initiale
                    </th>
                    {results.algorithm === "multilevel_dynamic" && (
                      <th scope="col" className="px-6 py-3">
                        Priorité Finale
                      </th>
                    )}
                    <th scope="col" className="px-6 py-3">
                      Temps de Fin
                    </th>
                    <th scope="col" className="px-6 py-3 rounded-tr-lg">
                      Temps d'Attente
                    </th>
                  </tr>
                </thead>
                <tbody>
                  {results.processStats.map((stat, index) => (
                    <tr key={stat.id} className="bg-white border-b hover:bg-gray-50 transition-colors">
                      <th scope="row" className="px-6 py-4 font-medium text-gray-900 whitespace-nowrap">
                        {stat.id}
                      </th>
                      <td className="px-6 py-4">{stat.arrivalTime}</td>
                      <td className="px-6 py-4">{stat.executionTime}</td>
                      <td className="px-6 py-4">{stat.priority}</td>
                      {results.algorithm === "multilevel_dynamic" && (
                        <td className="px-6 py-4 font-semibold text-orange-700">
                          {(stat as any).finalPriority !== undefined ? (stat as any).finalPriority : stat.priority}
                        </td>
                      )}
                      <td className="px-6 py-4 font-semibold text-green-700">{stat.finishTime}</td>
                      <td className="px-6 py-4 font-semibold text-blue-700">{stat.waitTime}</td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </CardContent>
        </Card>

        {}
        <Card className="bg-white/95 backdrop-blur-sm border-white/20 shadow-xl w-full">
          <CardHeader>
            <CardTitle className="text-black">Statistiques par Processus</CardTitle>
            <CardDescription className="text-gray-600">Temps d'attente et temps de rotation</CardDescription>
          </CardHeader>
          <CardContent>
            <ChartContainer
              config={{
                waitTime: {
                  label: "Temps d'attente",
                  color: WAIT_TIME_COLOR,
                },
                totalTime: {
                  label: "Temps de rotation",
                  color: TOTAL_TIME_COLOR,
                },
              }}
              className="h-[400px]"
            >
              <ResponsiveContainer width="100%" height="100%">
                <BarChart data={barData} margin={{ top: 20, right: 30, left: 20, bottom: 5 }}>
                  <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
                  <XAxis dataKey="name" stroke="#000000" />
                  <YAxis stroke="#000000" />
                  <ChartTooltip
                    content={<ChartTooltipContent />}
                    cursor={{ fill: "rgba(0, 0, 0, 0.1)" }}
                    formatter={(value: number) => [`${value} s`, ""]}
                  />
                  <Legend />
                  <Bar dataKey="Temps d'attente" fill={WAIT_TIME_COLOR} radius={[4, 4, 0, 0]} />
                  <Bar dataKey="Temps de rotation" fill={TOTAL_TIME_COLOR} radius={[4, 4, 0, 0]} />
                </BarChart>
              </ResponsiveContainer>
            </ChartContainer>
            <div className="mt-4 text-lg text-gray-800 border-t border-gray-100 pt-4 space-y-2 text-center font-semibold">
              <p>
                Temps de rotation moyen : <span className="text-2xl text-[#500010]">{averageTurnaroundTime.toFixed(2)} s</span>
              </p>
              <p>
                Temps d'attente moyen : <span className="text-2xl text-[#500010]">{averageWaitTime.toFixed(2)} s</span>
              </p>
            </div>
          </CardContent>
        </Card>

        {}
        <Card className="bg-white/95 backdrop-blur-sm border-white/20 shadow-xl w-full">
          <CardHeader>
            <CardTitle className="text-black">Distribution du Temps Total</CardTitle>
            <CardDescription className="text-gray-600">Répartition du temps total par processus</CardDescription>
          </CardHeader>
          <CardContent>
            <ChartContainer
              config={{
                value: {
                  label: "Temps",
                  color: "hsl(var(--color-chart-1))",
                },
              }}
              className="h-[400px]"
            >
              <ResponsiveContainer width="100%" height="100%">
                <PieChart>
                  <Pie
                    data={pieData}
                    cx="50%"
                    cy="50%"
                    labelLine={false}
                    label={({ name, percent }) => `${name} (${(percent * 100).toFixed(0)}%)`}
                    outerRadius={150} 
                    fill="#8884d8"
                    dataKey="value"
                  >
                    {pieData.map((entry, index) => (
                      <Cell key={`cell-${index}`} fill={entry.fill} />
                    ))}
                  </Pie>
                  <ChartTooltip
                    content={<ChartTooltipContent />}
                    formatter={(value: number) => [`${value} s`, "Temps d'exécution"]}
                  />
                </PieChart>
              </ResponsiveContainer>
            </ChartContainer>
            <div className="mt-6 text-center border-t border-gray-100 pt-4">
              <p className="text-gray-600 text-lg">
                Temps total du CPU :{" "}
                <span className="text-2xl font-bold text-[#500010]">{Number.isInteger(cpuTotalTime) ? cpuTotalTime : cpuTotalTime.toFixed(2)} s</span>
              </p>
            </div>
          </CardContent>
        </Card>

      </div>
    </div>
  )
}

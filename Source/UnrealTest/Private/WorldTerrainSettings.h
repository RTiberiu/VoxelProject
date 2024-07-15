#pragma once
#include <cstdint>
#include <condition_variable> 
#include <mutex> 
#include <atomic>

// World settings
static int UnrealScale{ 100 };
static std::atomic<int> DrawDistance{ 5 };

// Single chunk settings
static const uint16_t chunkHeight{ 248 }; // 4 bits 
static const uint8_t chunkSize{ 62 }; // 62
static const uint8_t chunkSizePadding{ 64 }; // 64
static const uint8_t intsPerHeight{ static_cast<uint8_t>(chunkHeight / chunkSize) };

// Player settings
static FVector playerInitialPosition;

// Locks for critical sections
static FCriticalSection PlayerPositionCriticalSection;
static FCriticalSection TickCriticalSection;
static FCriticalSection MapCriticalSection;
static FCriticalSection UpdateChunkCriticalSection;
static FCriticalSection DrawDistanceCriticalSection;

static FEvent* UpdateGameThreadEvent;

// Map to store spawned chunks with 2D coordinates as keys
static TMap<FIntPoint, AActor*> SpawnedChunksMap;



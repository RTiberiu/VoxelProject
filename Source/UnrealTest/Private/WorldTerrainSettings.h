// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <cstdint>
#include <condition_variable> 
#include <mutex> 
#include <atomic>

#include "FairSemaphore.h"

#include "CoreMinimal.h"
#include "WorldTerrainSettings.generated.h"

UCLASS()
class UWorldTerrainSettings : public UObject {
	GENERATED_BODY()

public:
	UWorldTerrainSettings();

	~UWorldTerrainSettings();

	const uint8_t UnrealScale{ 20 }; // this changes the voxel size (100 is 1m)
	const uint8_t DrawDistance{ 5 }; // 5 


	// Single chunk settings
	const uint16_t chunkHeight{ 248 }; // 4 bits
	const uint8_t chunkSize{ 62 }; // 62
	const uint8_t chunkSizePadding{ 64 }; // 64
	const uint8_t intsPerHeight{ static_cast<uint8_t>(chunkHeight / chunkSize) };

	// World chunks settings
	const int biomeWidth{ chunkSize * 5 }; // 10
	const uint64_t blendBiomeThreshold{ 150 };

	void ValidateSpawnedChunksMap();

	// Methods to interact with the world chunks
	void AddChunkToMap(const FIntPoint& ChunkCoordinates, AActor* ChunkActor);
	AActor* GetAndRemoveChunkFromMap(const FIntPoint& ChunkCoordinates);
	AActor* GetNextChunkFromMap();
	int GetMapSize();
	void updateInitialPlayerPosition(FVector newPosition);
	FVector getInitialPlayerPosition();
	void EmptyChunkMap();

	FairSemaphore* UpdateChunkSemaphore;

	bool isActorPresentInMap(AActor* actor); // TODO REMOVE AFTER TESTING - USING THIS BEFORE DESTROY() 

	void printMapElements(FString message); // TODO SHOULD MADE PRIVATE AFTER TESTING

	TArray<TArray<FColor>> ColorArray = {
        // Layer 1 (0-20 blocks)
    {
        FColor(64, 48, 33), FColor(72, 54, 38), FColor(60, 45, 28), FColor(68, 52, 36),
        FColor(70, 50, 30), FColor(62, 46, 32), FColor(66, 49, 35), FColor(65, 47, 31),
        FColor(73, 55, 39), FColor(67, 51, 33), FColor(63, 49, 30)
    },
        // Layer 2 (20-40 blocks)
        {
            FColor(85, 65, 42), FColor(90, 70, 45), FColor(80, 60, 40), FColor(88, 68, 44),
            FColor(87, 66, 41), FColor(83, 62, 39), FColor(82, 63, 43), FColor(86, 69, 47),
            FColor(84, 64, 41), FColor(89, 67, 42), FColor(81, 63, 38)
        },
        // Layer 3 (40-60 blocks)
        {
            FColor(125, 94, 64), FColor(130, 98, 68), FColor(120, 90, 62), FColor(128, 92, 65),
            FColor(126, 95, 66), FColor(124, 91, 60), FColor(127, 96, 63), FColor(122, 93, 67),
            FColor(131, 99, 69), FColor(123, 89, 61), FColor(129, 97, 64)
        },
        // Layer 4 (60-80 blocks)
        {
            FColor(165, 138, 89), FColor(170, 142, 93), FColor(160, 135, 88), FColor(167, 140, 90),
            FColor(162, 137, 86), FColor(168, 143, 91), FColor(164, 139, 87), FColor(169, 144, 92),
            FColor(163, 136, 85), FColor(171, 141, 94), FColor(166, 137, 89)
        },
        // Layer 5 (80-100 blocks)
        {
            FColor(106, 148, 67), FColor(110, 150, 70), FColor(104, 145, 65), FColor(107, 149, 68),
            FColor(108, 152, 69), FColor(105, 147, 64), FColor(109, 151, 66), FColor(103, 146, 63),
            FColor(111, 153, 71), FColor(102, 144, 62), FColor(106, 150, 72)
        },
        // Layer 6 (100-120 blocks)
        {
            FColor(80, 120, 40), FColor(82, 122, 42), FColor(78, 118, 38), FColor(81, 121, 41),
            FColor(83, 123, 43), FColor(79, 119, 37), FColor(84, 124, 44), FColor(77, 117, 36),
            FColor(85, 125, 45), FColor(76, 116, 35), FColor(80, 120, 46)
        },
        // Layer 7 (120-140 blocks)
        {
            FColor(140, 170, 220), FColor(145, 175, 225), FColor(135, 165, 215), FColor(142, 172, 218),
            FColor(138, 168, 217), FColor(146, 174, 223), FColor(139, 169, 216), FColor(144, 173, 221),
            FColor(137, 167, 214), FColor(148, 176, 226), FColor(141, 171, 219)
        },
        // Layer 8 (140-160 blocks)
        {
            FColor(110, 160, 230), FColor(115, 165, 235), FColor(105, 155, 225), FColor(112, 162, 232),
            FColor(111, 158, 228), FColor(116, 166, 237), FColor(107, 157, 224), FColor(114, 164, 231),
            FColor(106, 156, 223), FColor(117, 167, 238), FColor(110, 159, 229)
        },
        // Layer 9 (160-180 blocks)
        {
            FColor(190, 225, 245), FColor(195, 230, 250), FColor(185, 220, 240), FColor(192, 228, 247),
            FColor(189, 223, 242), FColor(196, 231, 249), FColor(187, 221, 239), FColor(194, 229, 248),
            FColor(186, 219, 238), FColor(198, 233, 251), FColor(191, 224, 246)
        },
        // Layer 10 (180-200 blocks)
        {
            FColor(255, 250, 250), FColor(255, 252, 252), FColor(254, 249, 249), FColor(253, 251, 253),
            FColor(255, 249, 250), FColor(254, 253, 255), FColor(255, 248, 248), FColor(252, 250, 252),
            FColor(255, 251, 249), FColor(253, 252, 254), FColor(255, 250, 251)
        }
	};

private:
	void CheckForDuplicateActorPointers();
	void CheckNumberOfElements();
	void CheckIfActorIsNullOrPendingKill();

	// Player settings
	FVector playerInitialPosition;

	// Locks for critical sections
	FairSemaphore* PlayerPositionSemaphore;
	FairSemaphore* MapSemaphore;
	FairSemaphore* DrawDistanceSemaphore;

	// Map to store spawned chunks with 2D coordinates as keys
	TMap<FIntPoint, AActor*> SpawnedChunksMap;
};

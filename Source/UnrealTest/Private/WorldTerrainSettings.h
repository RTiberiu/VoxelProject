// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <cstdint>
#include <condition_variable> 
#include <mutex> 
#include <atomic>

#include "FairSemaphore.h"
#include "CoreMinimal.h"
#include "WorldTerrainSettings.generated.h"

class FastNoiseLite;
class APerlinNoiseSettings;

UCLASS()
class UWorldTerrainSettings : public UObject {
	GENERATED_BODY()

public:
	UWorldTerrainSettings();

	~UWorldTerrainSettings();

	const uint8_t UnrealScale{ 50 }; // this changes the voxel size (100 is 1m)
	const uint8_t DrawDistance{ 25 }; // 5 

	// Single chunk settings
	const uint16_t chunkHeight{ 248 }; // 4 bits
	const uint8_t chunkSize{ 62 }; // 62
	const uint8_t chunkSizePadding{ 64 }; // 64
	const uint8_t intsPerHeight{ static_cast<uint8_t>(chunkHeight / chunkSize) };

	// World chunks settings
    const int CollisionDistance{ chunkSize * UnrealScale * 2};

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
    const TMap<FIntPoint, AActor*>& GetSpawnedChunksMap() const; 

	FairSemaphore* UpdateChunkSemaphore;

    // Chunk noises
    // Noise objects used to generate the world
    TObjectPtr<FastNoiseLite> continentalness;
    TObjectPtr<FastNoiseLite> erosion;
    TObjectPtr<FastNoiseLite> peaksAndValleys;

    // Domain warp for the 3 main noise objects
    TObjectPtr<FastNoiseLite> continentalnessDW;
    TObjectPtr<FastNoiseLite> erosionDW;
    TObjectPtr<FastNoiseLite> peaksAndValleysDW;

    void SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings);

    void ApplyDomainWarpToCoords(float& noisePositionX, float& noisePositionZ, TObjectPtr<FastNoiseLite> noise);

    float GetNoiseAtCoords(float& noisePositionX, float& noisePositionZ, TObjectPtr<FastNoiseLite> noise);

	bool isActorPresentInMap(AActor* actor); // TODO REMOVE AFTER TESTING - USING THIS BEFORE DESTROY() 

	void printMapElements(FString message); // TODO SHOULD MADE PRIVATE AFTER TESTING

    TArray<TArray<FColor>> ColorArray = {
        // Layer 1 (0-20 blocks) - Dark Brown
        {
            FColor(45, 41, 32), FColor(46, 42, 33), FColor(44, 40, 31), FColor(47, 43, 34),
            FColor(48, 44, 35), FColor(46, 42, 32), FColor(45, 41, 30), FColor(43, 39, 29),
            FColor(50, 46, 37), FColor(46, 44, 32), FColor(44, 39, 30)
        },
        // Layer 2 (20-40 blocks) - Lighter Brown
        {
            FColor(79, 69, 55), FColor(81, 71, 57), FColor(77, 67, 53), FColor(78, 68, 54),
            FColor(80, 70, 56), FColor(76, 66, 52), FColor(82, 72, 58), FColor(75, 65, 51),
            FColor(83, 73, 59), FColor(74, 64, 50), FColor(77, 67, 53)
        },
        // Layer 3 (40-60 blocks) - Light Brown
        {
            FColor(105, 87, 59), FColor(107, 88, 60), FColor(103, 85, 58), FColor(104, 86, 59),
            FColor(106, 87, 60), FColor(102, 84, 57), FColor(108, 89, 61), FColor(101, 83, 56),
            FColor(109, 90, 62), FColor(100, 82, 55), FColor(104, 86, 58)
        },
        // Layer 4 (60-80 blocks) - Dark Green
        {
            FColor(31, 38, 15), FColor(21, 28, 5), FColor(41, 48, 25), FColor(31, 35, 13),
            FColor(31, 40, 18), FColor(36, 43, 20), FColor(26, 33, 10), FColor(31, 38, 17),
            FColor(26, 38, 15), FColor(41, 48, 30), FColor(31, 38, 19)
        },
        // Layer 5 (80-100 blocks) - Lighter Green
        {
            FColor(78, 89, 39), FColor(68, 79, 29), FColor(88, 99, 49), FColor(78, 86, 36),
            FColor(78, 91, 41), FColor(83, 94, 43), FColor(73, 84, 33), FColor(78, 89, 38),
            FColor(73, 89, 39), FColor(88, 99, 54), FColor(78, 89, 42)
        },
        // Layer 6 (100-120 blocks) - Light Green
        {
             FColor(110, 121, 69), FColor(100, 111, 59), FColor(120, 131, 79), FColor(110, 116, 66),
            FColor(110, 124, 71), FColor(115, 126, 73), FColor(105, 116, 63), FColor(110, 121, 68),
            FColor(105, 121, 69), FColor(120, 131, 84), FColor(110, 121, 70)
        },
        // Layer 7 (120-140 blocks) - Dark Grey
        {
            FColor(63, 63, 63), FColor(53, 53, 53), FColor(73, 73, 73), FColor(63, 58, 58),
            FColor(63, 68, 68), FColor(68, 68, 68), FColor(58, 58, 58), FColor(63, 63, 62),
            FColor(58, 63, 63), FColor(73, 73, 68), FColor(63, 63, 63)
        },
        // Layer 8 (140-160 blocks) - Lighter Grey
        {
            FColor(89, 89, 89), FColor(79, 79, 79), FColor(99, 99, 99), FColor(89, 84, 84),
            FColor(89, 94, 94), FColor(94, 94, 94), FColor(84, 84, 84), FColor(89, 89, 88),
            FColor(84, 89, 89), FColor(99, 99, 94), FColor(89, 89, 90)
        },
        // Layer 9 (160-180 blocks) - Light Grey
        {
            FColor(140, 140, 140), FColor(130, 130, 130), FColor(150, 150, 150), FColor(140, 135, 135),
            FColor(140, 145, 145), FColor(145, 145, 145), FColor(135, 135, 135), FColor(140, 140, 139),
            FColor(135, 140, 140), FColor(150, 150, 145), FColor(140, 140, 140)
        },
        // Layer 10 (180-200 blocks) - White
        {
            FColor(230, 225, 210), FColor(232, 227, 212), FColor(228, 224, 208), FColor(229, 225, 209),
            FColor(231, 226, 211), FColor(227, 223, 207), FColor(233, 228, 213), FColor(226, 222, 206),
            FColor(234, 229, 214), FColor(225, 221, 205), FColor(229, 225, 210)
        }
    };

private:
    APerlinNoiseSettings* PerlinNoiseSettingsRef;
    APerlinNoiseSettings*& PNSR = PerlinNoiseSettingsRef;

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
    

    void initializePerlinNoise(TObjectPtr<FastNoiseLite>& noise);

    void applyPerlinNoiseSettings(TObjectPtr<FastNoiseLite>& noise, const int& settingsIndex);

    void initializeDomainWarpNoise(TObjectPtr<FastNoiseLite>& domainWarp);

    void applyDomainWarpSettings(TObjectPtr<FastNoiseLite>& domainWarp, const int& settingsIndex);
};

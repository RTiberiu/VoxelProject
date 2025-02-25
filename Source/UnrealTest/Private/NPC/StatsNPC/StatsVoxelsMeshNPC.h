#pragma once

#include "CoreMinimal.h"
#include "..\..\Chunks\DataStructures\VoxelObjectMeshData.h"
#include "..\..\Chunks\DataStructures\VoxelObjectLocationData.h"
#include "StatsVoxelsMeshNPC.generated.h"

enum StatsType {
	Stamina,
	Hunger,
	HealthPoints,
	FoodPouch,
	AlliesInPack
};



UCLASS()
class UStatsVoxelsMeshNPC : public UObject {
	GENERATED_BODY()
	
public:
	UStatsVoxelsMeshNPC();
	~UStatsVoxelsMeshNPC();
	const TArray<StatsType>& GetStatsTypes();
	const FVector& GetStatsVoxelOffset(StatsType statsType);

	void AddStatsMeshData(const StatsType& InStatsType, const int& InStatsFill, FVoxelObjectMeshData InStatsMeshData);
	FVoxelObjectMeshData* GetStatsMeshData(const StatsType& InStatsType, const int& InStatsFill);

private:
	const TArray<StatsType> StatTypes = {
		StatsType::Stamina,
		StatsType::Hunger,
		StatsType::HealthPoints,
		StatsType::FoodPouch,
		StatsType::AlliesInPack
	};

	const TMap<StatsType, FVector> StatsVoxelOffsets = {
		{StatsType::Stamina, FVector(0.0f, -75.0f, 150.0f)},
		{StatsType::Hunger, FVector(0.0f, -25.0f, 150.0f)},
		{StatsType::HealthPoints, FVector(0.0f, 25.0f, 150.0f)},
		{StatsType::FoodPouch, FVector(0.0f, -50.0f, 200.0f)},
		{StatsType::AlliesInPack, FVector(0.0f, 0.0f, 200.0f)}
	};

	
	// For each type of stats, store each version of the voxel mesh data (from 1-27)
	TMap<StatsType, TMap<int, FVoxelObjectMeshData>> StatsMeshData;

};

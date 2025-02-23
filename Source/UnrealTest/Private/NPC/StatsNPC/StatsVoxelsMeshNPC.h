#pragma once

#include "CoreMinimal.h"
#include "..\..\Chunks\DataStructures\VoxelObjectMeshData.h"
#include "..\..\Chunks\DataStructures\VoxelObjectLocationData.h"


enum StatsType {
	Stamina,
	Hunger,
	HealthPoints,
	FoodPouch,
	Allies
};

UCLASS()
class UStatsVoxelsMeshNPC : public UObject {
	GENERATED_BODY()
	
public:
	UStatsVoxelsMeshNPC();
	~UStatsVoxelsMeshNPC();

	void AddStatsMeshData(const StatsType& InStatsType, const int& InStatsFill, FVoxelObjectMeshData* InStatsMeshData);
	FVoxelObjectMeshData* GetStatsMeshData(const StatsType& InStatsType, const int& InStatsFill);
private:
	
	// For each type of stats, store each version of the voxel mesh data (from 1-9)
	TMap<StatsType, TMap<int, FVoxelObjectMeshData*>> StatsMeshData;

};

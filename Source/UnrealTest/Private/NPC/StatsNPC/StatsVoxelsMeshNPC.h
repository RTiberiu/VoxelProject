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
	AlliesInPack,
	Notification
};



UCLASS()
class UStatsVoxelsMeshNPC : public UObject {
	GENERATED_BODY()
	
public:
	UStatsVoxelsMeshNPC();
	~UStatsVoxelsMeshNPC();
	const TArray<StatsType>& GetStatsTypes();
	const TMap<int, TArray<FColor>>& GetStatsColors();
	const FVector& GetStatsVoxelOffset(StatsType statsType);

	void AddStatsMeshData(const StatsType& InStatsType, const int& InStatsFill, FVoxelObjectMeshData InStatsMeshData);
	FVoxelObjectMeshData* GetStatsMeshData(const StatsType& InStatsType, const int& InStatsFill);

private:
	const TArray<StatsType> StatTypes = {
		StatsType::Stamina,
		StatsType::Hunger,
		StatsType::HealthPoints,
		StatsType::FoodPouch,
		StatsType::AlliesInPack,
		StatsType::Notification
	};

	const TMap<StatsType, FVector> StatsVoxelOffsets = {
		{StatsType::Stamina, FVector(50.0f, -100.0f, 150.0f)},
		{StatsType::Hunger, FVector(50.0f, -50.0f, 150.0f)},
		{StatsType::HealthPoints, FVector(50.0f, 0.0f, 150.0f)},
		{StatsType::FoodPouch, FVector(50.0f, -75.0f, 200.0f)},
		{StatsType::AlliesInPack, FVector(50.0f, -25.0f, 200.0f)},
		{StatsType::Notification, FVector(50.0f, -50.0f, 250.0f)}
	};

	const TMap<int, TArray<FColor>> StatsColors = {
	{0, {FColor(143, 227, 255), FColor(133, 224, 255), FColor(122, 222, 255), FColor(112, 219, 255), FColor(102, 217, 255), FColor(92, 214, 255), FColor(82, 212, 255), FColor(71, 209, 255), FColor(61, 207, 255), FColor(51, 204, 255), FColor(41, 201, 255), FColor(31, 199, 255)}},
	{1, {FColor(255, 253, 143), FColor(255, 253, 133), FColor(255, 253, 122), FColor(255, 253, 112), FColor(255, 252, 102), FColor(255, 252, 92), FColor(255, 252, 82), FColor(255, 252, 71), FColor(255, 252, 61), FColor(255, 252, 51), FColor(255, 251, 41), FColor(255, 251, 31)}},
	{2, {FColor(255, 167, 143), FColor(255, 159, 133), FColor(255, 151, 122), FColor(255, 143, 112), FColor(255, 135, 102), FColor(255, 127, 92), FColor(255, 119, 82), FColor(255, 111, 71), FColor(255, 103, 61), FColor(255, 95, 51), FColor(255, 87, 41), FColor(255, 79, 31)}},
	{3, {FColor(203, 143, 255), FColor(198, 133, 255), FColor(193, 122, 255), FColor(188, 112, 255), FColor(184, 102, 255), FColor(179, 92, 255), FColor(174, 82, 255), FColor(169, 71, 255), FColor(165, 61, 255), FColor(160, 51, 255), FColor(155, 41, 255), FColor(150, 31, 255)}},
	{4, {FColor(143, 255, 197), FColor(133, 255, 192), FColor(122, 255, 186), FColor(112, 255, 181), FColor(102, 255, 176), FColor(92, 255, 171), FColor(82, 255, 165), FColor(71, 255, 160), FColor(61, 255, 155), FColor(51, 255, 150), FColor(41, 255, 144), FColor(31, 255, 139)}},
	{5, {FColor(68,68,68), FColor(104,7,187), FColor(78,176,19), FColor(196,35,35)}}
	};

	
	// For each type of stats, store each version of the voxel mesh data (from 1-27)
	TMap<StatsType, TMap<int, FVoxelObjectMeshData>> StatsMeshData;

};

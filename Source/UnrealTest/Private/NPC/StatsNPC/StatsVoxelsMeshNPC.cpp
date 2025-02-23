#include "StatsVoxelsMeshNPC.h"

UStatsVoxelsMeshNPC::UStatsVoxelsMeshNPC() {
}

UStatsVoxelsMeshNPC::~UStatsVoxelsMeshNPC() {
}

void UStatsVoxelsMeshNPC::AddStatsMeshData(const StatsType& InStatsType, const int& InStatsFill, FVoxelObjectMeshData* InStatsMeshData) {
	if (StatsMeshData.Contains(InStatsType)) {
		StatsMeshData[InStatsType].Add(InStatsFill, InStatsMeshData);
	} else {
		TMap<int, FVoxelObjectMeshData*> StatsFillMeshData;
		StatsFillMeshData.Add(InStatsFill, InStatsMeshData);
		StatsMeshData.Add(InStatsType, StatsFillMeshData);
	}
}

FVoxelObjectMeshData* UStatsVoxelsMeshNPC::GetStatsMeshData(const StatsType& InStatsType, const int& InStatsFill) {
	return StatsMeshData[InStatsType][InStatsFill];
}

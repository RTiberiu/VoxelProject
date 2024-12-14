#pragma once

#include "CoreMinimal.h"
#include "VoxelObjectMeshData.generated.h"

USTRUCT()
struct FVoxelObjectMeshData {
	GENERATED_BODY()

public:
	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector> Normals;
	TArray<FColor> Colors;
	TArray<FVector2D> UV0;
};

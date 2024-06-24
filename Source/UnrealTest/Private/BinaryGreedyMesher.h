#pragma once

#include "CoreMinimal.h"
#include "Voxel.h"

class BinaryGreedyMesher {
public:
    TArray<FQuad> Mesh(const TArray<FVoxel>& Voxels, int SizeX, int SizeY, int SizeZ);

private:
    int GetVoxel(const TArray<FVoxel>& Voxels, int X, int Y, int Z, int SizeX, int SizeY, int SizeZ, int Axis);
};

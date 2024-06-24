#include "BinaryGreedyMesher.h"

TArray<FQuad> BinaryGreedyMesher::Mesh(const TArray<FVoxel>& Voxels, int SizeX, int SizeY, int SizeZ) {
    TArray<FQuad> Quads;

    // Process each axis (XY, YZ, XZ)
    for (int Axis = 0; Axis < 3; ++Axis) {
        TArray<int32> Mask;
        Mask.SetNumZeroed(SizeX * SizeY);

        for (int Z = 0; Z < SizeZ; ++Z) {
            for (int Y = 0; Y < SizeY; ++Y) {
                for (int X = 0; X < SizeX; ++X) {
                    int Current = GetVoxel(Voxels, X, Y, Z, SizeX, SizeY, SizeZ, Axis);
                    Mask[Y * SizeX + X] = Current ? 1 : 0;
                }
            }

            for (int Y = 0; Y < SizeY; ++Y) {
                for (int X = 0; X < SizeX;) {
                    if (Mask[Y * SizeX + X]) {
                        int Width = 1;
                        while (X + Width < SizeX && Mask[Y * SizeX + X + Width]) {
                            ++Width;
                        }

                        int Height = 1;
                        while (Y + Height < SizeY) {
                            bool Expand = true;
                            for (int I = 0; I < Width; ++I) {
                                if (!Mask[(Y + Height) * SizeX + X + I]) {
                                    Expand = false;
                                    break;
                                }
                            }
                            if (!Expand) break;
                            ++Height;
                        }

                        for (int I = 0; I < Height; ++I) {
                            for (int J = 0; J < Width; ++J) {
                                Mask[(Y + I) * SizeX + X + J] = 0;
                            }
                        }

                        FQuad Quad = { X, Y, Z, Width, Height, Axis };
                        Quads.Add(Quad);
                        X += Width;
                    } else {
                        ++X;
                    }
                }
            }
        }
    }

    return Quads;
}

int BinaryGreedyMesher::GetVoxel(const TArray<FVoxel>& Voxels, int X, int Y, int Z, int SizeX, int SizeY, int SizeZ, int Axis) {
    int Index;
    switch (Axis) {
    case 0: Index = X + Y * SizeX + Z * SizeX * SizeY; break;
    case 1: Index = Y + Z * SizeY + X * SizeY * SizeZ; break;
    case 2: Index = Z + X * SizeZ + Y * SizeZ * SizeX; break;
    default: Index = 0; break;
    }
    return Voxels.IsValidIndex(Index) ? Voxels[Index].bSolid : 0;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "GreedyChunk.h"

#include "Enums.h"
#include "ProceduralMeshComponent.h"
#include "FastNoiseLite.h"

// Sets default values
AGreedyChunk::AGreedyChunk() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");

	Noise = new FastNoiseLite();
	Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise->SetFractalType(FastNoiseLite::FractalType_FBm);

	// ReSize array depending on chunk Size
	Blocks.SetNum(Size.X * Size.Y * MaxHeight);

	Mesh->SetCastShadow(false);

	// Set mesh as root
	SetRootComponent(Mesh);

}

// Called when the game starts or when spawned
void AGreedyChunk::BeginPlay() {
	Super::BeginPlay();

	GenerateBlocks();

	GenerateMesh();

	ApplyMesh();

}

void AGreedyChunk::GenerateBlocks() {
	const FVector Location = GetActorLocation();

	for (int x = 0; x < Size.X; x++) {
		for (int y = 0; y < Size.Y; y++) {
			const float Xpos = (x * UnrealScale + Location.X) / UnrealScale;
			const float Ypos = (y * UnrealScale + Location.Y) / UnrealScale;

			// Looping over the different octaves to get the final height
			int height = 0;
			int amplitude = 0;
			for (const float octaveFrequency : OctavesFrequency) {
				Noise->SetFrequency(octaveFrequency);
				const float noiseValue = Noise->GetNoise(Xpos, Ypos) + 1;

				// Adding multiple splines to the perlinValue
				if (noiseValue <= 1) {
					amplitude = 15; // 15
				} else if (noiseValue <= 1.4) {
					amplitude = 20; // 20
				} else if (noiseValue <= 1.8) {
					amplitude = 30; // 30
				} else if (noiseValue <= 2) {
					amplitude = 25; // 25
				}

				height += std::floor(noiseValue * amplitude);
			}

			height = FMath::Clamp(height, 0, MaxHeight);

			for (int z = 0; z < height; z++) {
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
			}

			for (int z = height; z < MaxHeight; z++) {
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
			}
		}
	}
}

void AGreedyChunk::GenerateMesh() {
	// Sweep over each axis (X, Y, Z)
	for (int Axis = 0; Axis < 3; ++Axis) {
		// 2 Perpendicular axis
		const int Axis1 = (Axis + 1) % 3;
		const int Axis2 = (Axis + 2) % 3;
		
		const int MainAxisLimit = Size[Axis];
		const int Axis1Limit = Size[Axis1];
		const int Axis2Limit = Size[Axis2];

		FIntVector DeltaAxis1 = FIntVector::ZeroValue;
		FIntVector DeltaAxis2 = FIntVector::ZeroValue;

		FIntVector ChunkItr = FIntVector::ZeroValue;
		FIntVector AxisMask = FIntVector::ZeroValue;

		AxisMask[Axis] = 1;

		TArray<FMask> Mask;
		Mask.SetNum(Axis1Limit * Axis2Limit);

		// Check each slice of the chunk
		for (ChunkItr[Axis] = -1; ChunkItr[Axis] < MainAxisLimit;) {
			int N = 0;

			// Compute Mask
			for (ChunkItr[Axis2] = 0; ChunkItr[Axis2] < Axis2Limit; ++ChunkItr[Axis2]) {
				for (ChunkItr[Axis1] = 0; ChunkItr[Axis1] < Axis1Limit; ++ChunkItr[Axis1]) {
					const auto CurrentBlock = GetBlock(ChunkItr);
					const auto CompareBlock = GetBlock(ChunkItr + AxisMask);

					const bool CurrentBlockOpaque = CurrentBlock != EBlock::Air;
					const bool CompareBlockOpaque = CompareBlock != EBlock::Air;

					if (CurrentBlockOpaque == CompareBlockOpaque) {
						Mask[N++] = FMask{ EBlock::Null, 0 };
					} else if (CurrentBlockOpaque) {
						Mask[N++] = FMask{ CurrentBlock, 1 };
					} else {
						Mask[N++] = FMask{ CompareBlock, -1 };
					}
				}
			}

			++ChunkItr[Axis];
			N = 0;

			// Generate Mesh From Mask
			for (int j = 0; j < Axis2Limit; ++j) {
				for (int i = 0; i < Axis1Limit;) {
					if (Mask[N].Normal != 0) {
						const auto CurrentMask = Mask[N];
						ChunkItr[Axis1] = i;
						ChunkItr[Axis2] = j;

						int Width;

						for (Width = 1; i + Width < Axis1Limit && isMaskTheSame(Mask[N + Width], CurrentMask); ++Width) {
						}

						int Height;
						bool Done = false;

						for (Height = 1; j + Height < Axis2Limit; ++Height) {
							for (int k = 0; k < Width; ++k) {
								if (isMaskTheSame(Mask[N + k + Height * Axis1Limit], CurrentMask)) continue;

								Done = true;
								break;
							}

							if (Done) break;
						}

						DeltaAxis1[Axis1] = Width;
						DeltaAxis2[Axis2] = Height;

						CreateQuad(
							CurrentMask, AxisMask, Width, Height,
							ChunkItr,
							ChunkItr + DeltaAxis1,
							ChunkItr + DeltaAxis2,
							ChunkItr + DeltaAxis1 + DeltaAxis2
						);

						DeltaAxis1 = FIntVector::ZeroValue;
						DeltaAxis2 = FIntVector::ZeroValue;

						for (int l = 0; l < Height; ++l) {
							for (int k = 0; k < Width; ++k) {
								Mask[N + k + l * Axis1Limit] = FMask{ EBlock::Null, 0 };
							}
						}

						i += Width;
						N += Width;
					} else {
						i++;
						N++;
					}
				}
			}
		}
	}
}

int AGreedyChunk::GetTextureIndex(const EBlock Block, const FVector Normal) const {
	switch (Block) {
	case EBlock::Grass:
	{
		if (Normal == FVector::UpVector) return 0;
		return 1;
	}
	case EBlock::Dirt: return 2;
	case EBlock::Stone: return 3;
	default: return 255;
	}
}

void AGreedyChunk::CreateQuad(
	const FMask Mask,
	const FIntVector AxisMask,
	const int Width,
	const int Height,
	const FIntVector V1,
	const FIntVector V2,
	const FIntVector V3,
	const FIntVector V4
) {
	const auto Normal = FVector(AxisMask * Mask.Normal);
	const auto Color = FColor(0, 0, 0, GetTextureIndex(Mask.Block, Normal));

	MeshData.Vertices.Append({
		FVector(V1) * 100,
		FVector(V2) * 100,
		FVector(V3) * 100,
		FVector(V4) * 100
		});

	MeshData.Triangles.Append({
		VertexCount,
		VertexCount + 2 + Mask.Normal,
		VertexCount + 2 - Mask.Normal,
		VertexCount + 3,
		VertexCount + 1 - Mask.Normal,
		VertexCount + 1 + Mask.Normal
		});

	MeshData.Normals.Append({
		Normal,
		Normal,
		Normal,
		Normal
		});

	MeshData.Colors.Append({
		Color,
		Color,
		Color,
		Color
		});

	if (Normal.X == 1 || Normal.X == -1) {
		MeshData.UV0.Append({
			FVector2D(Width, Height),
			FVector2D(0, Height),
			FVector2D(Width, 0),
			FVector2D(0, 0),
			});
	} else {
		MeshData.UV0.Append({
			FVector2D(Height, Width),
			FVector2D(Height, 0),
			FVector2D(0, Width),
			FVector2D(0, 0),
			});
	}

	VertexCount += 4;
}

void AGreedyChunk::ApplyMesh() {
	Mesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, TArray<FVector>(), MeshData.UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
}

int AGreedyChunk::GetBlockIndex(int X, int Y, int Z) const {
	return Z * Size.X * Size.Y + Y * Size.X + X;
}

EBlock AGreedyChunk::GetBlock(FIntVector Index) const {
	// TODO Check faces between chunks too, so that faces covered by other chunks won't be spawned

	// If outside of the current chunck consider it as Air 
	if (Index.X >= Size.X || Index.Y >= Size.Y || Index.Z >= MaxHeight || Index.X < 0 || Index.Y < 0 || Index.Z < 0) {
		return EBlock::Air;
	}

	return Blocks[GetBlockIndex(Index.X, Index.Y, Index.Z)];
}

bool AGreedyChunk::isMaskTheSame(FMask M1, FMask M2) const {
	return M1.Block == M2.Block && M1.Normal == M2.Normal;
}


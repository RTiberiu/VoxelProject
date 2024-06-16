// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"

#include "Enums.h"
#include "FastNoiseLite.h"
#include "ProceduralMeshComponent.h"
#include <stdexcept>

// Sets default values
AChunk::AChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");

	Noise = new FastNoiseLite();
	Noise->SetFrequency(0.03f);
	Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise->SetFractalType(FastNoiseLite::FractalType_FBm);

	// Resize array depending on chunk size
	Blocks.SetNum(size * size * size);

	Mesh->SetCastShadow(false);
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();

	GenerateBlocks();

	GenerateMesh();

	ApplyMesh();
}

void AChunk::GenerateBlocks() {

	const auto Location = GetActorLocation();

	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			const float Xpos = (x * 100 + Location.X) / 100;
			const float Ypos = (y * 100 + Location.Y) / 100;

			const int Height = FMath::Clamp(FMath::RoundToInt((Noise->GetNoise(Xpos, Ypos) + 1) * size / 2), 0, size);

			for (int z = 0; z < Height; z++) {
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
			}

			for (int z = Height; z < size; z++) {
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
			}
		}
	}
}

void AChunk::GenerateMesh() {
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			for (int z = 0; z < size; z++) {
				if (Blocks[GetBlockIndex(x, y, z)] != EBlock::Air) {
					const auto Position = FVector(x, y, z);

					for (auto Direction : { EDirection::Forward, EDirection::Right, EDirection::Back, EDirection::Left, EDirection::Up, EDirection::Down }) {
						if (Check(GetPositionInDirection(Direction, Position))) {
							CreateFace(Direction, Position * 100);
						}
					}
				}
			}

		}
	}
}

void AChunk::ApplyMesh() const {
	Mesh->CreateMeshSection(0, VertexData, TriangleData, TArray<FVector>(), UVData, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
}

void AChunk::CreateFace(EDirection Direction, FVector Position) {
	VertexData.Append(GetFaceVertices(Direction, Position));
	UVData.Append({ FVector2D(1, 1), FVector2D(1, 0), FVector2D(0, 0), FVector2D(0, 1) });
	TriangleData.Append({ VertexCount + 3, VertexCount + 2, VertexCount , VertexCount + 2, VertexCount + 1, VertexCount });
	VertexCount += 4;
}


TArray<FVector> AChunk::GetFaceVertices(EDirection Direction, FVector Position) const {
	TArray<FVector> Vertices;

	for(int i = 0; i < 4; i++) {
		Vertices.Add(BlockVertexData[BlockTriangleData[i + static_cast<int>(Direction) * 4]] * scale + Position);
	}

	return Vertices;
}

// Check if the current position contains an Air block
bool AChunk::Check(FVector Position) const {
	// If outside of the current chunck consider it as Air 
	if (Position.X >= size || Position.Y >= size || Position.Z >= size || Position.X < 0 || Position.Y < 0 || Position.Z < 0) {
		return true;
	}

	return Blocks[GetBlockIndex(Position.X, Position.Y, Position.Z)] == EBlock::Air;
}

FVector AChunk::GetPositionInDirection(EDirection Direction, FVector Position) const {
	switch (Direction) {
		case EDirection::Forward: return Position + FVector::ForwardVector;
		case EDirection::Right: return Position + FVector::RightVector;
		case EDirection::Back: return Position + FVector::BackwardVector;
		case EDirection::Left: return Position + FVector::LeftVector;
		case EDirection::Up: return Position + FVector::UpVector;
		case EDirection::Down: return Position + FVector::DownVector;
		default: throw std::invalid_argument("Invalid direction");
	}
}

// Access 3D coordinates in a 1D 
int AChunk::GetBlockIndex(int x, int y, int z) const {
	return z * size * size + y * size + x;
}


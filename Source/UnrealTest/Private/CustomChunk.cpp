// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomChunk.h"

#include "Enums.h"
#include "FastNoiseLite.h"
#include "ProceduralMeshComponent.h"
#include <stdexcept>

// Sets default values
ACustomChunk::ACustomChunk() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");

	Noise = new FastNoiseLite();
	Noise->SetFrequency(0.03f);
	Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise->SetFractalType(FastNoiseLite::FractalType_FBm);

	// ReSize array depending on chunk Size
	Blocks.SetNum(Size * Size * MaxHeight);

	Mesh->SetCastShadow(false);
}

// Called when the game starts or when spawned
void ACustomChunk::BeginPlay() {
	Super::BeginPlay();

	GenerateBlocks();

	GenerateMesh();

	ApplyMesh();
}

void ACustomChunk::GenerateBlocks() {

	const auto Location = GetActorLocation();

	for (int x = 0; x < Size; x++) {
		for (int y = 0; y < Size; y++) {
			const float Xpos = (x * 100 + Location.X) / 100;
			const float Ypos = (y * 100 + Location.Y) / 100;

			// Looping over the different octaves to get the final height
			int height = 0;
			int amplitude = 0;
			for (const float octaveFrequency : OctavesFrequency) {
				Noise->SetFrequency(octaveFrequency);
				const float noiseValue = Noise->GetNoise(Xpos, Ypos) + 1;
 
				// Adding multiple splines to the perlinValue
				if (noiseValue <= 1) {
					amplitude = 15;
				} else if (noiseValue <= 1.4) {
					amplitude = 20;
				} else if (noiseValue <= 1.8) {
					amplitude = 30;
				} else if (noiseValue <= 2) {
					amplitude = 25;
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

void ACustomChunk::GenerateMesh() {
	for (int x = 0; x < Size; x++) {
		for (int y = 0; y < Size; y++) {
			for (int z = 0; z < MaxHeight; z++) {
				if (Blocks[GetBlockIndex(x, y, z)] != EBlock::Air) {
					const FVector Position = FVector(x, y, z);

					for (EDirection Direction : { EDirection::Forward, EDirection::Right, EDirection::Back, EDirection::Left, EDirection::Up, EDirection::Down }) {
						if (Check(GetPositionInDirection(Direction, Position))) {
							CreateFace(Direction, Position * 100);
						}
					}
				}
			}

		}
	}
}

void ACustomChunk::ApplyMesh() const {
	Mesh->CreateMeshSection(0, VertexData, TriangleData, TArray<FVector>(), UVData, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
}

void ACustomChunk::CreateFace(EDirection Direction, FVector Position) {
	VertexData.Append(GetFaceVertices(Direction, Position));
	UVData.Append({ FVector2D(1, 1), FVector2D(1, 0), FVector2D(0, 0), FVector2D(0, 1) });
	TriangleData.Append({ VertexCount + 3, VertexCount + 2, VertexCount , VertexCount + 2, VertexCount + 1, VertexCount });
	VertexCount += 4;
}


TArray<FVector> ACustomChunk::GetFaceVertices(EDirection Direction, FVector Position) const {
	TArray<FVector> Vertices;

	for (int i = 0; i < 4; i++) {
		Vertices.Add(BlockVertexData[BlockTriangleData[i + static_cast<int>(Direction) * 4]] * Scale + Position);
	}

	return Vertices;
}

// Check if the current position contains an Air block
bool ACustomChunk::Check(FVector Position) const {
	// If outside of the current chunck consider it as Air 
	if (Position.X >= Size || Position.Y >= Size || Position.Z >= MaxHeight || Position.X < 0 || Position.Y < 0 || Position.Z < 0) {
		return true;
	}

	return Blocks[GetBlockIndex(Position.X, Position.Y, Position.Z)] == EBlock::Air;
}

FVector ACustomChunk::GetPositionInDirection(EDirection Direction, FVector Position) const {
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
int ACustomChunk::GetBlockIndex(int x, int y, int z) const {
	return z * Size * Size + y * Size + x;
}


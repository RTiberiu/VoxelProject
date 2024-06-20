// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BinaryChunk.generated.h"

class FastNoiseLite;
class UProceduralMeshComponent;

UCLASS()
class ABinaryChunk : public AActor {
	GENERATED_BODY()

	// Store solid blocks in a 3D chunk of 8 x 8 x chunkHeight
	// X = Array Index: 0 to chunkSize - 1
	// Z = Array Index: chunkSize to (chunkSize * chunkSize)
	// Y (Height) = Bit Index
	struct BinaryChunk3D {
		// Size = chunkSize * chunkSize * chunkHeight
		std::vector<uint64_t> voxelsPosition;
	};

	struct ChunkMesh {
		std::vector<uint64_t> indices;
		std::vector<uint64_t> vertices;
	};

public:
	// Sets default values for this actor's properties
	ABinaryChunk();

	void populateChunkVertices();

private: 
	TObjectPtr<UProceduralMeshComponent> Mesh;
	TObjectPtr<FastNoiseLite> noise;

	BinaryChunk3D binaryChunk = BinaryChunk3D{};
	
	const uint16_t chunkHeight{ 256 }; // 4 bits (320 for 5 bits)
	const uint8_t chunkSize{ 64 };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};

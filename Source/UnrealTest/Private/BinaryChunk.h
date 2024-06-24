// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <chrono>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BinaryChunk.generated.h"

class FastNoiseLite;
class UProceduralMeshComponent;

UCLASS()
class ABinaryChunk : public AActor {
	GENERATED_BODY()

	// Store solid blocks in a 3D chunk of chunkSize * chunkSize * intsPerHeight
	struct BinaryChunk3D {
		std::vector<uint64_t> yBinaryColumn;
		std::vector<uint64_t> xBinaryColumn;
		std::vector<uint64_t> zBinaryColumn;
	};

	struct ChunkMesh {
		std::vector<uint64_t> indices;
		std::vector<uint64_t> vertices;
	};

public:
	// Sets default values for this actor's properties
	ABinaryChunk();


private: 
	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	TObjectPtr<UProceduralMeshComponent> Mesh;
	TObjectPtr<FastNoiseLite> noise;

	BinaryChunk3D binaryChunk = BinaryChunk3D{};

	std::vector<uint64_t> columnsFaceMask;
	
	const uint16_t chunkHeight{ 256 }; // 4 bits (320 for 5 bits)
	const uint8_t chunkSize{ 64 };
	const uint8_t intsPerHeight{ static_cast<uint8_t>(chunkHeight / chunkSize) };

	void createBinarySolidColumnsYXZ();

	void faceCullingBinaryColumnsYXZ();

	// void generateChunkMesh();

	void printExecutionTime(Time& start, Time& end, const char* functionName);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};

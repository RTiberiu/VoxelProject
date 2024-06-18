// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkMeshData.h"
#include "GameFramework/Actor.h"
#include "GreedyChunk.generated.h"

enum class EBlock;
class FastNoiseLite;
class UProceduralMeshComponent;

UCLASS()
class AGreedyChunk : public AActor
{
	GENERATED_BODY()

	struct FMask {
		EBlock Block;
		int Normal;
	};
	
public:	
	// Sets default values for this actor's properties
	AGreedyChunk();

	UPROPERTY(EditAnywhere, Category = "Greedy Chunk")
	int ChunkSize{ 32 };

	UPROPERTY(EditAnywhere, Category = "Greedy Chunk")
	FIntVector Size = FIntVector(1, 1, 1) * ChunkSize;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:
	TObjectPtr<UProceduralMeshComponent> Mesh;
	TObjectPtr<FastNoiseLite> Noise;

	FChunkMeshData MeshData;
	TArray<EBlock> Blocks;

	int VertexCount{ 0 };

	void GenerateBlocks();

	void ApplyMesh();

	void CreateQuad(FMask Mask, FIntVector AxisMask, FIntVector V1, FIntVector V2, FIntVector V3, FIntVector V4);

	int GetBlockIndex(int X, int Y, int Z) const;

	EBlock GetBlock(FIntVector Index) const;

	bool isMaskTheSame(FMask M1, FMask M2) const; // CompareMask in the tutorial

};

// Fill out your copyright notice in the Description page of Project Settings.


#include "GreedyChunk.h"

// Sets default values
AGreedyChunk::AGreedyChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AGreedyChunk::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGreedyChunk::GenerateBlocks() {
}

void AGreedyChunk::ApplyMesh() {
}

void AGreedyChunk::CreateQuad(FMask Mask, FIntVector AxisMask, FIntVector V1, FIntVector V2, FIntVector V3, FIntVector V4) {
}

int AGreedyChunk::GetBlockIndex(int X, int Y, int Z) const {
	return 0;
}

EBlock AGreedyChunk::GetBlock(FIntVector Index) const {
	return EBlock();
}

bool AGreedyChunk::isMaskTheSame(FMask M1, FMask M2) const {
	return false;
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomChunkWorld.h"

// Sets default values
ACustomChunkWorld::ACustomChunkWorld() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACustomChunkWorld::BeginPlay() {
	Super::BeginPlay();

	for (int x = -drawDistance; x < drawDistance; x++) {
		for (int y = -drawDistance; y < drawDistance; y++) {
			GetWorld()->SpawnActor<AActor>(chunk, FVector(x * chunkSize * 100, y * chunkSize * 100, 0), FRotator::ZeroRotator);
		}
	}
}


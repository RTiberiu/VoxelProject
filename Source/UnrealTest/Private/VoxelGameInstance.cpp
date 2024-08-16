// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelGameInstance.h"
#include <Kismet/GameplayStatics.h>
#include "Engine/World.h"
#include "GameFramework/Actor.h" 

// Sets default values
UVoxelGameInstance::UVoxelGameInstance() {
   
}

void UVoxelGameInstance::ShutDown() {
    Super::Shutdown();

    // Removing added objects from root
    if (worldTerrainSettings) {
        worldTerrainSettings->RemoveFromRoot();
        worldTerrainSettings = nullptr;
    }

    if (chunkLocationData) {
        chunkLocationData->RemoveFromRoot();
        chunkLocationData = nullptr;
    }
}

void UVoxelGameInstance::Init() {
	Super::Init();

    chunkLocationData = NewObject<UChunkLocationData>();
    worldTerrainSettings = NewObject<UWorldTerrainSettings>();

    // Ensuring there is not premature garbage collection on the object
    worldTerrainSettings->AddToRoot();
    chunkLocationData->AddToRoot();

    // Spawn chunkWorld as an actor in the world
    FActorSpawnParameters SpawnParams;

    UE_LOG(LogTemp, Warning, TEXT("Attempting to spawn Chunk World!"));

    TSubclassOf<AActor> ChunkWorldClass = AChunkWorld::StaticClass();

    // Get the player controller as the owner of AChunkWorld
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    AActor* OwnerActor = PlayerController ? PlayerController->GetPawn() : nullptr;

    chunkWorld = GetWorld()->SpawnActorDeferred<AChunkWorld>(ChunkWorldClass, FTransform(FRotator::ZeroRotator, FVector::ZeroVector), OwnerActor, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (chunkWorld) {
        // Set the chunk world references to terrain settings and chunk location data
        chunkWorld->SetChunkLocationData(chunkLocationData);
        chunkWorld->SetWorldTerrainSettings(worldTerrainSettings);

        // Finish spawning the chunk world
        UGameplayStatics::FinishSpawningActor(chunkWorld, FTransform(FRotator::ZeroRotator, FVector::ZeroVector));

        UE_LOG(LogTemp, Warning, TEXT("Spawned Chunk World!"));
    }
   
}




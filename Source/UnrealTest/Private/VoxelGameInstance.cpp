// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelGameInstance.h"
#include <Kismet/GameplayStatics.h>
#include "Engine/World.h"
#include "GameFramework/Actor.h" 

// Sets default values
AVoxelGameInstance::AVoxelGameInstance() {
    PrimaryActorTick.bCanEverTick = false;
}

void AVoxelGameInstance::BeginPlay() {
	Super::BeginPlay();

    if (GEngine) {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("VoxelGameInstance Initialized!"));
    }

    chunkLocationData = NewObject<UChunkLocationData>();
    worldTerrainSettings = NewObject<UWorldTerrainSettings>();

    // Ensuring there is not premature garbage collection on the object
    worldTerrainSettings->AddToRoot();
    chunkLocationData->AddToRoot();

    // Spawn PerlinNoiseSettings for allowing value changes in the editor
    perlinNoiseSettings = GetWorld()->SpawnActor<APerlinNoiseSettings>(
            APerlinNoiseSettings::StaticClass(),
            FVector::ZeroVector,                  
            FRotator::ZeroRotator               
    );
    
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
        chunkWorld->SetPerlinNoiseSettings(perlinNoiseSettings);
        chunkWorld->InitializePathfindingManager();

        // Finish spawning the chunk world
        UGameplayStatics::FinishSpawningActor(chunkWorld, FTransform(FRotator::ZeroRotator, FVector::ZeroVector));

        UE_LOG(LogTemp, Warning, TEXT("Spawned Chunk World!"));
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("Spawned Chunk World!"));
    }
}

void AVoxelGameInstance::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    // Removing added objects from root
    if (worldTerrainSettings) {
        worldTerrainSettings->RemoveFromRoot();
        worldTerrainSettings = nullptr;
    }

    if (chunkLocationData) {
        chunkLocationData->RemoveFromRoot();
        chunkLocationData = nullptr;
    }

    if (chunkWorld) {
        chunkWorld->RemoveFromRoot();
        chunkWorld->Destroy();
        chunkWorld = nullptr;
    }

    if (perlinNoiseSettings) {
        perlinNoiseSettings->RemoveFromRoot();
        perlinNoiseSettings->Destroy();
        perlinNoiseSettings = nullptr;
    }
}




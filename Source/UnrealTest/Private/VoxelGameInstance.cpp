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
    animationSettingsNpcRef = NewObject<UAnimationSettingsNPC>();
    statsVoxelsMeshNPC = NewObject<UStatsVoxelsMeshNPC>();

    // Add a world terrain settings ref to chunk location data
    chunkLocationData->SetWorldTerrainSettings(worldTerrainSettings);

    // Ensuring there is not premature garbage collection on the object
    worldTerrainSettings->AddToRoot();
    chunkLocationData->AddToRoot();
    animationSettingsNpcRef->AddToRoot();
    statsVoxelsMeshNPC->AddToRoot();

    // Spawn PerlinNoiseSettings for allowing value changes in the editor
    perlinNoiseSettings = GetWorld()->SpawnActor<APerlinNoiseSettings>(
            APerlinNoiseSettings::StaticClass(),
            FVector::ZeroVector,                  
            FRotator::ZeroRotator               
    );

    // Generate the stats voxel meshes used for NPCs to show their attributes
    GenerateStatsVoxelMeshes();

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
        chunkWorld->SetAnimationSettingsNpc(animationSettingsNpcRef);
        chunkWorld->InitializePathfindingManager();
		chunkWorld->SetStatsVoxelsMeshNPC(statsVoxelsMeshNPC);

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

    if (animationSettingsNpcRef) {
        animationSettingsNpcRef->RemoveFromRoot();
        animationSettingsNpcRef = nullptr;
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

void AVoxelGameInstance::GenerateStatsVoxelMeshes() {
    UStatsMeshGenerator* statsMeshGenerator = NewObject<UStatsMeshGenerator>();

    const int VoxelFillAmount = 27;
	const TArray<StatsType> StatTypes = statsVoxelsMeshNPC->GetStatsTypes();
    const TMap<int, TArray<FColor>> StatsColors = statsVoxelsMeshNPC->GetStatsColors();

    for (int i = 1; i <= VoxelFillAmount; i++) {
		for (int j = 0; j < StatTypes.Num(); j++) {
            const TArray<FColor>& Colors = StatsColors[j];

            // Reduce the number of voxel fill amount for the notification voxel 
            // (since only one is needed, with 3 color variations)
            bool isNotificationStat = StatTypes[j] == StatsType::Notification;
            if (isNotificationStat && i > Colors.Num()) {
                continue;
            }

            // Mapping the i index to the colors index
            int ColorIndex = FMath::RoundToInt(((i - 1) / static_cast<float>(VoxelFillAmount - 1)) * (Colors.Num() - 1));

            int CurrentFillAmount;
            if (isNotificationStat) {
                CurrentFillAmount = VoxelFillAmount;
                ColorIndex = i - 1;
            } else {
                CurrentFillAmount = i;
            }

            FColor SelectedColor = Colors[ColorIndex];

			FVoxelObjectMeshData meshData = statsMeshGenerator->GetStatsMeshData(SelectedColor, CurrentFillAmount);
			statsVoxelsMeshNPC->AddStatsMeshData(StatTypes[j], i, meshData);
		}
    }
}




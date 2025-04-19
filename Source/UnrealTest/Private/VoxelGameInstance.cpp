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
    const TMap<int, TArray<FColor>> StatsColors = {
        {0, {FColor(143, 227, 255), FColor(133, 224, 255), FColor(122, 222, 255), FColor(112, 219, 255), FColor(102, 217, 255), FColor(92, 214, 255), FColor(82, 212, 255), FColor(71, 209, 255), FColor(61, 207, 255), FColor(51, 204, 255), FColor(41, 201, 255), FColor(31, 199, 255)}},
        {1, {FColor(255, 253, 143), FColor(255, 253, 133), FColor(255, 253, 122), FColor(255, 253, 112), FColor(255, 252, 102), FColor(255, 252, 92), FColor(255, 252, 82), FColor(255, 252, 71), FColor(255, 252, 61), FColor(255, 252, 51), FColor(255, 251, 41), FColor(255, 251, 31)}},
        {2, {FColor(255, 167, 143), FColor(255, 159, 133), FColor(255, 151, 122), FColor(255, 143, 112), FColor(255, 135, 102), FColor(255, 127, 92), FColor(255, 119, 82), FColor(255, 111, 71), FColor(255, 103, 61), FColor(255, 95, 51), FColor(255, 87, 41), FColor(255, 79, 31)}},
        {3, {FColor(203, 143, 255), FColor(198, 133, 255), FColor(193, 122, 255), FColor(188, 112, 255), FColor(184, 102, 255), FColor(179, 92, 255), FColor(174, 82, 255), FColor(169, 71, 255), FColor(165, 61, 255), FColor(160, 51, 255), FColor(155, 41, 255), FColor(150, 31, 255)}},
        {4, {FColor(143, 255, 197), FColor(133, 255, 192), FColor(122, 255, 186), FColor(112, 255, 181), FColor(102, 255, 176), FColor(92, 255, 171), FColor(82, 255, 165), FColor(71, 255, 160), FColor(61, 255, 155), FColor(51, 255, 150), FColor(41, 255, 144), FColor(31, 255, 139)}},
    };

    for (int i = 1; i <= VoxelFillAmount; i++) {
		for (int j = 0; j < StatTypes.Num(); j++) {
            const TArray<FColor>& Colors = StatsColors[j];

            // Mapping the i index to the colors index
            int ColorIndex = FMath::RoundToInt(((i - 1) / static_cast<float>(VoxelFillAmount - 1)) * (Colors.Num() - 1));
            FColor SelectedColor = Colors[ColorIndex];

			FVoxelObjectMeshData meshData = statsMeshGenerator->GetStatsMeshData(SelectedColor, i);
			statsVoxelsMeshNPC->AddStatsMeshData(StatTypes[j], i, meshData);
		}
    }
}




// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "WorldTerrainSettings.h"
#include "ChunkLocationData.h"
#include "BinaryChunk.h" 
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AChunkWorld::AChunkWorld() : terrainRunnable(nullptr), terrainRunnableThread(nullptr), isTaskRunning(false) {
	// Set this actor to call Tick() every frame.  Yosu can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	isInitialWorldGenerated = false;

	// Initializing Chunk with the BinaryChunk class
	Chunk = ABinaryChunk::StaticClass();
}

void AChunkWorld::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void AChunkWorld::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}

void AChunkWorld::SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings) {
	PerlinNoiseSettingsRef = InPerlinNoiseSettings;
}

void AChunkWorld::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

void AChunkWorld::spawnInitialWorld() {
	int spawnedChunks{ 0 };
	Time start = std::chrono::high_resolution_clock::now();

	for (int x = -WTSR->DrawDistance; x < WTSR->DrawDistance; x++) {
		for (int z = -WTSR->DrawDistance; z < WTSR->DrawDistance; z++) {
			FVector ChunkPosition = FVector(x * WTSR->chunkSize * WTSR->UnrealScale, z * WTSR->chunkSize * WTSR->UnrealScale, 0);

			// Spawn the chunk actor deferred
			ABinaryChunk* SpawnedChunkActor = GetWorld()->SpawnActorDeferred<ABinaryChunk>(Chunk, FTransform(FRotator::ZeroRotator, ChunkPosition), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			if (SpawnedChunkActor) {
					// Add WorldTerrainSettingsRef to BinaryChunk
					SpawnedChunkActor->SetWorldTerrainSettings(WTSR);
					SpawnedChunkActor->SetPerlinNoiseSettings(PNSR);

					// Finish spawning the chunk actor
					UGameplayStatics::FinishSpawningActor(SpawnedChunkActor, FTransform(FRotator::ZeroRotator, ChunkPosition));

					WTSR->AddChunkToMap(FIntPoint(x, z), SpawnedChunkActor);

					spawnedChunks++;
			} else {
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn Chunk Actor!"));
			}
		}
	}

	Time end = std::chrono::high_resolution_clock::now();

	printExecutionTime(start, end, "Spawned entire terrain.");
	UE_LOG(LogTemp, Warning, TEXT("Chunks spawned: %d"), spawnedChunks);
	UE_LOG(LogTemp, Warning, TEXT("SpawnedChunkMap size = %d"), WTSR->GetMapSize());
	UE_LOG(LogTemp, Warning, TEXT("DrawDistance = %d"), WTSR->DrawDistance);
}

// Perform any actions after generating the new chunks
void AChunkWorld::onNewTerrainGenerated() {
}

void AChunkWorld::destroyCurrentWorldChunks() {
	bool isWorldEmpty = false;

	while (!isWorldEmpty) {
		AActor* chunkToRemove = WTSR->GetNextChunkFromMap();
		if (chunkToRemove) {
			chunkToRemove->Destroy();
		} else {
			// Empty indices for chunks to spawn and destroy
			CLDR->emptyPositionQueues();

			isWorldEmpty = true;
		}
	}
}

// Called when the game starts or when spawned
void AChunkWorld::BeginPlay() {
	Super::BeginPlay();

	spawnInitialWorld();

	// Set player's initial position
	WTSR->updateInitialPlayerPosition(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation());

	// Set the player's maximum speed 
	ADefaultPawn* PlayerPawn = Cast<ADefaultPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (PlayerPawn) {
		UFloatingPawnMovement* MovementComponent = Cast<UFloatingPawnMovement>(PlayerPawn->GetMovementComponent());
		if (MovementComponent) {
			MovementComponent->MaxSpeed = 30000.0f;
			UE_LOG(LogTemp, Warning, TEXT("Player speed set to %f"), MovementComponent->MaxSpeed);
		} else {
			UE_LOG(LogTemp, Error, TEXT("Player pawn does not have a movement component!"));
		}
	} else {
		UE_LOG(LogTemp, Error, TEXT("Player pawn is not a DefaultPawn!"));
	}

	isInitialWorldGenerated = true;

	WTSR->printMapElements("Map after BeginPlay()");
}

FIntPoint AChunkWorld::GetChunkCoordinates(FVector Position) const {
	int32 ChunkX = FMath::FloorToInt(Position.X / (WTSR->chunkSize * WTSR->UnrealScale));
	int32 ChunkZ = FMath::FloorToInt(Position.Y / (WTSR->chunkSize * WTSR->UnrealScale));
	return FIntPoint(ChunkX, ChunkZ);
}

// Called every frame
void AChunkWorld::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	// If Perlin noise settings changed, respawn the world
	if (PNSR->changedSettings) {
		isInitialWorldGenerated = false;

		destroyCurrentWorldChunks();

		spawnInitialWorld();

		isInitialWorldGenerated = true;

		PNSR->changedSettings = false;
	}

	// Continue running only if the BeginPlay() is done initializing the world
	if (!isInitialWorldGenerated) {
		UE_LOG(LogTemp, Warning, TEXT("World not yet initialized. Tick() will exit now."));
		return;
	}

	if (WTSR == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("WTSR is nullptr!"));
		return;
	}

	const FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

	const FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	const FIntPoint InitialChunkCoords = GetChunkCoordinates(WTSR->getInitialPlayerPosition());

	const bool isPlayerMovingOnAxisX = PlayerChunkCoords.X != InitialChunkCoords.X;
	const bool isPlayerMovingOnAxisZ = PlayerChunkCoords.Y != InitialChunkCoords.Y;

	if (!isTaskRunning && (isPlayerMovingOnAxisX || isPlayerMovingOnAxisZ)) {
		isTaskRunning = true;
		terrainRunnable = new TerrainRunnable(PlayerPosition, WTSR, CLDR);
		terrainRunnableThread = FRunnableThread::Create(terrainRunnable, TEXT("terrainRunnableThread"), 0, TPri_Normal);
	}

	if (terrainRunnable && terrainRunnable->IsTaskComplete()) {
		onNewTerrainGenerated();

		// Clean up
		if (terrainRunnableThread) {
			terrainRunnableThread->Kill(true);
			delete terrainRunnableThread;
			terrainRunnableThread = nullptr;
		}
		if (terrainRunnable) {
			delete terrainRunnable;
			terrainRunnable = nullptr;
		}

		isTaskRunning = false;
	}

	// Spawn and destroy one chunk if there is one waiting
	FChunkLocationData chunkToSpawnPosition;
	const bool doesSpawnPositionExist = CLDR->getChunkToSpawnPosition(chunkToSpawnPosition);

	if (doesSpawnPositionExist) {
		Time startChunkTest = std::chrono::high_resolution_clock::now(); // TODO TESTING SINGLE CHUNK SPAWN TIME

		// Spawn the chunk actor deferred
		ABinaryChunk* SpawnedChunkActor = GetWorld()->SpawnActorDeferred<ABinaryChunk>(Chunk, FTransform(FRotator::ZeroRotator, chunkToSpawnPosition.ChunkPosition), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (SpawnedChunkActor) {
				// Add WTSR to BinaryChunk
				SpawnedChunkActor->SetWorldTerrainSettings(WTSR);
				SpawnedChunkActor->SetPerlinNoiseSettings(PNSR);


				// Finish spawning the chunk actor
				UGameplayStatics::FinishSpawningActor(SpawnedChunkActor, FTransform(FRotator::ZeroRotator, chunkToSpawnPosition.ChunkPosition));

				WTSR->AddChunkToMap(chunkToSpawnPosition.ChunkWorldCoords, SpawnedChunkActor);

		} else {
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn Chunk Actor!"));
		}

		// Calculating the average time to spawn a chunk
		Time endChunkTest = std::chrono::high_resolution_clock::now();
		calculateAverageChunkSpawnTime(startChunkTest, endChunkTest);
	}

	FIntPoint chunkToDestroyPosition;
	bool doesDestroyPositionExist = CLDR->getChunkToDestroyPosition(chunkToDestroyPosition);

	if (doesDestroyPositionExist) {
		AActor* chunkToRemove = WTSR->GetAndRemoveChunkFromMap(chunkToDestroyPosition);
		if (chunkToRemove) {
			chunkToRemove->Destroy();
		}
	}
}

void AChunkWorld::calculateAverageChunkSpawnTime(const Time& startTime, const Time& endTime) {
	std::chrono::duration<double, std::milli> duration = endTime - startTime;
	double chunkSpawnTime = duration.count();

	// Accumulate time and increment chunk count
	TotalTimeForChunks += chunkSpawnTime;
	ChunksSpawnedCount++;

	// Check if enough chunks spawned to calculate the average
	if (ChunksSpawnedCount >= ChunksToAverage) {
		double averageTime = TotalTimeForChunks / ChunksSpawnedCount;
		UE_LOG(LogTemp, Warning, TEXT("Average time to spawn a BinaryChunk: %f milliseconds; Calculated after spawning %d chunks."), averageTime, ChunksSpawnedCount);

		// Reset counters
		ChunksSpawnedCount = 0;
		TotalTimeForChunks = 0.0;
	}
}
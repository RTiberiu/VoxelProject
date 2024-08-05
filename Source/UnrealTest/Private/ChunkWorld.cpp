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
	PrimaryActorTick.bCanEverTick = true; // true; testing spawning a single chunk for now. put to true for infinite world

	isInitialWorldGenerated = false;

	// Initializing Chunk with the BinaryChunk class
	Chunk = ABinaryChunk::StaticClass();

	// TerrainRunnable::setDrawDistance(5);
}

void AChunkWorld::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void AChunkWorld::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}

void AChunkWorld::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

void AChunkWorld::spawnInitialWorld() {
	int spawnedChunks{ 0 };
	Time start = std::chrono::high_resolution_clock::now();

	for (int x = -WorldTerrainSettingsRef->DrawDistance; x < WorldTerrainSettingsRef->DrawDistance; x++) { // for (int x = -WorldTerrainSettingsRef->DrawDistance; x < WorldTerrainSettingsRef->DrawDistance; x++) {
		for (int z = -WorldTerrainSettingsRef->DrawDistance; z < WorldTerrainSettingsRef->DrawDistance; z++) { // for (int z = -WorldTerrainSettingsRef->DrawDistance; z < WorldTerrainSettingsRef->DrawDistance; z++) {
			FVector ChunkPosition = FVector(x * WorldTerrainSettingsRef->chunkSize * WorldTerrainSettingsRef->UnrealScale, z * WorldTerrainSettingsRef->chunkSize * WorldTerrainSettingsRef->UnrealScale, 0);

			// ABinaryChunk* SpawnedChunkActor = GetWorld()->SpawnActor<ABinaryChunk>(Chunk, ChunkPosition, FRotator::ZeroRotator);

			// Spawn the chunk actor deferred
			ABinaryChunk* SpawnedChunkActor = GetWorld()->SpawnActorDeferred<ABinaryChunk>(Chunk, FTransform(FRotator::ZeroRotator, ChunkPosition), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			if (SpawnedChunkActor) {
					// Add WorldTerrainSettingsRef to BinaryChunk
					SpawnedChunkActor->SetWorldTerrainSettings(WorldTerrainSettingsRef);

					// Finish spawning the chunk actor
					UGameplayStatics::FinishSpawningActor(SpawnedChunkActor, FTransform(FRotator::ZeroRotator, ChunkPosition));

					WorldTerrainSettingsRef->AddChunkToMap(FIntPoint(x, z), SpawnedChunkActor);

					spawnedChunks++;
			} else {
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn Chunk Actor!"));
			}
		}
	}

	Time end = std::chrono::high_resolution_clock::now();

	printExecutionTime(start, end, "Spawned entire terrain.");
	UE_LOG(LogTemp, Warning, TEXT("Chunks spawned: %d"), spawnedChunks);
	UE_LOG(LogTemp, Warning, TEXT("SpawnedChunkMap size = %d"), WorldTerrainSettingsRef->GetMapSize());
	UE_LOG(LogTemp, Warning, TEXT("DrawDistance = %d"), WorldTerrainSettingsRef->DrawDistance);

	// Print SpawnedChunksMap keys before destroying
	WorldTerrainSettingsRef->printMapElements("SpawnedChunksMap Keys AFTER spawnInitialWorld");
}



// Perform any actions after generating the new chunks
void AChunkWorld::onNewTerrainGenerated() {
}

// Called when the game starts or when spawned
void AChunkWorld::BeginPlay() {
	Super::BeginPlay();

	spawnInitialWorld();

	// Set player's initial position
	WorldTerrainSettingsRef->updateInitialPlayerPosition(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation());

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
}

FIntPoint AChunkWorld::GetChunkCoordinates(FVector Position) const {
	int32 ChunkX = FMath::FloorToInt(Position.X / (WorldTerrainSettingsRef->chunkSize * WorldTerrainSettingsRef->UnrealScale));
	int32 ChunkY = FMath::FloorToInt(Position.Y / (WorldTerrainSettingsRef->chunkSize * WorldTerrainSettingsRef->UnrealScale));
	return FIntPoint(ChunkX, ChunkY);
}

// Called every frame
void AChunkWorld::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	// Continue running only if the BeginPlay() is done initializing the world
	if (!isInitialWorldGenerated) {
		UE_LOG(LogTemp, Warning, TEXT("World not yet initialized. Tick() will exit now."));
		return;
	}

	// TODO There are still some crashes when moving at high speed in the world. 
	FScopeLock Lock(&WorldTerrainSettingsRef->TickCriticalSection);
	
	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

	FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	FIntPoint InitialChunkCoords = GetChunkCoordinates(WorldTerrainSettingsRef->getInitialPlayerPosition());

	bool isPlayerMovingOnAxisX = PlayerChunkCoords.X != InitialChunkCoords.X;
	bool isPlayerMovingOnAxisZ = PlayerChunkCoords.Y != InitialChunkCoords.Y;


	if (!isTaskRunning && (isPlayerMovingOnAxisX || isPlayerMovingOnAxisZ)) {
		isTaskRunning = true;

		UE_LOG(LogTemp, Warning, TEXT("Created new thread!"));

		terrainRunnable = new TerrainRunnable(PlayerPosition, WorldTerrainSettingsRef, ChunkLocationDataRef);
		terrainRunnableThread = FRunnableThread::Create(terrainRunnable, TEXT("terrainRunnableThread"), 0, TPri_Normal);
	}

	if (terrainRunnable && terrainRunnable->IsTaskComplete()) {
		UE_LOG(LogTemp, Warning, TEXT("Destroyed new thread!"));

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
	bool isSpawnPositionReturned = ChunkLocationDataRef->getChunkToSpawnPosition(chunkToSpawnPosition);

	if (isSpawnPositionReturned) {

		// Only proceed if a valid ChunkToSpawnPosition was dequeued
		if (!chunkToSpawnPosition.ChunkPosition.IsZero()) {

			Time startChunkTest = std::chrono::high_resolution_clock::now(); // TODO TESTING SINGLE CHUNK SPAWN TIME

			// Spawn the chunk actor deferred
			ABinaryChunk* SpawnedChunkActor = GetWorld()->SpawnActorDeferred<ABinaryChunk>(Chunk, FTransform(FRotator::ZeroRotator, chunkToSpawnPosition.ChunkPosition), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			if (SpawnedChunkActor) {

					// Add WorldTerrainSettingsRef to BinaryChunk
					SpawnedChunkActor->SetWorldTerrainSettings(WorldTerrainSettingsRef);

					// Finish spawning the chunk actor
					UGameplayStatics::FinishSpawningActor(SpawnedChunkActor, FTransform(FRotator::ZeroRotator, chunkToSpawnPosition.ChunkPosition));

					WorldTerrainSettingsRef->AddChunkToMap(chunkToSpawnPosition.ChunkWorldCoords, SpawnedChunkActor);

			} else {
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn Chunk Actor!"));
			}

			Time endChunkTest = std::chrono::high_resolution_clock::now(); // TODO TESTING SINGLE CHUNK SPAWN TIME
			printExecutionTime(startChunkTest, endChunkTest, "Spawned a single chunk.");

		}
	}

	FIntPoint chunkToDestroyPosition;
	bool isDestroyPositionReturned = ChunkLocationDataRef->getChunkToDestroyPosition(chunkToDestroyPosition);

	if (isDestroyPositionReturned) {
		AActor* chunkToRemove = WorldTerrainSettingsRef->GetAndRemoveChunkFromMap(chunkToDestroyPosition);
		if (chunkToRemove) {
			chunkToRemove->Destroy();
		}
	}

}


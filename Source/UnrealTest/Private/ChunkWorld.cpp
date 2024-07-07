// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
AChunkWorld::AChunkWorld() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AChunkWorld::UpdateChunks() {
	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

	 // Log the player's position
    // UE_LOG(LogTemp, Warning, TEXT("Player position: X=%f, Y=%f, Z=%f"), PlayerPosition.X, PlayerPosition.Y, PlayerPosition.Z);

	FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	FIntPoint InitialChunkCoords = GetChunkCoordinates(playerInitialPosition);

	// Add and remove chunks on the X axis 
	if (PlayerChunkCoords.X != InitialChunkCoords.X) {

		int lastRowX{ 0 };
		int newRowX { 0 };
		if (PlayerChunkCoords.X > InitialChunkCoords.X) {
			// Get the first row by adding the draw distance to the player's X
			lastRowX = PlayerChunkCoords.X - DrawDistance;
			newRowX = PlayerChunkCoords.X + DrawDistance;
			UE_LOG(LogTemp, Warning, TEXT("MINUS drawsitance for lastRowX"));
		} else {
			// Get the last row by substracting the draw distance from the the initial X
			lastRowX = InitialChunkCoords.X + DrawDistance;
			newRowX = InitialChunkCoords.X - DrawDistance;
			UE_LOG(LogTemp, Warning, TEXT("PLUS drawsitance for lastRowX"));
		}

		// Add new chunks and remove old chunks based on the player's new X position
		UE_LOG(LogTemp, Warning, TEXT("Player moved on X axis to chunk: X=%d"), PlayerChunkCoords.X);

		// Print SpawnedChunksMap keys before destroying
		int testCounter{ 0 };
		FString keysString = "SpawnedChunksMap Keys BEFORE X update: \n";
		for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {
			keysString += FString::Printf(TEXT("\tX=%d, Z=%d "), Pair.Key.X, Pair.Key.Y);
			testCounter++;

			if (testCounter == DrawDistance * 2) {
				keysString += TEXT("\n");
				testCounter = 0;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("%s"), *keysString);

		int firstIndexChunkZ = InitialChunkCoords.Y - DrawDistance;
		int lastIndexChunkZ = InitialChunkCoords.Y + DrawDistance;

		UE_LOG(LogTemp, Warning, TEXT("Attempting to get: X=%d - Z=%d to Z=%d"), lastRowX, firstIndexChunkZ, lastIndexChunkZ);
		UE_LOG(LogTemp, Warning, TEXT("InitialChunkCoords: X=%d Z=%d "), InitialChunkCoords.X, InitialChunkCoords.Y);
		UE_LOG(LogTemp, Warning, TEXT("PlayerChunkCoords: X=%d Z=%d "), PlayerChunkCoords.X, PlayerChunkCoords.Y);
		UE_LOG(LogTemp, Warning, TEXT("Adding to row X=%d"), newRowX);

		// Loop and remove the entire row of chunks 
		for (int z = firstIndexChunkZ; z < lastIndexChunkZ; z++) {
			AActor* currentChunk = SpawnedChunksMap.FindAndRemoveChecked(FIntPoint(lastRowX, z));
			currentChunk->Destroy();

			// Adding new row 
			FVector ChunkPosition = FVector(newRowX * ChunkSize * UnrealScale, z * ChunkSize * UnrealScale, 0);
			AActor* SpawnedChunk = GetWorld()->SpawnActor<AActor>(Chunk, ChunkPosition, FRotator::ZeroRotator);
			if (SpawnedChunk) {
				SpawnedChunksMap.Add(FIntPoint(newRowX, z), SpawnedChunk);
			}
		}

		// Print SpawnedChunksMap keys after adding
		testCounter = 0;
		keysString = "SpawnedChunksMap Keys AFTER X update: \n";
		for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {
			keysString += FString::Printf(TEXT("\tX=%d, Z=%d "), Pair.Key.X, Pair.Key.Y);
			testCounter++;

			if (testCounter == DrawDistance * 2) {
				keysString += TEXT("\n");
				testCounter = 0;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("%s"), *keysString);
	}

	// Update the initial position for the next frame
	PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	InitialChunkCoords = GetChunkCoordinates(playerInitialPosition);

	// Add and remove chunks on the Y axis 
	if (PlayerChunkCoords.Y != InitialChunkCoords.Y) { 
		// Add new chunks and remove old chunks based on the player's new Y position
		UE_LOG(LogTemp, Warning, TEXT("Player moved on Z axis to chunk: Z=%d"), PlayerChunkCoords.Y);

		// Print SpawnedChunksMap keys before destroying
		int testCounter{ 0 };
		FString keysString = "SpawnedChunksMap Keys BEFORE Z update: \n";
		for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {
			keysString += FString::Printf(TEXT("\tX=%d, Z=%d "), Pair.Key.X, Pair.Key.Y);
			testCounter++;

			if (testCounter == DrawDistance * 2) {
				keysString += TEXT("\n");
				testCounter = 0;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("%s"), *keysString);

		int lastRowZ{ 0 };
		int newRowZ{ 0 };
		if (PlayerChunkCoords.Y > InitialChunkCoords.Y) {
			// Get the first row by adding the draw distance to the player's Z
			UE_LOG(LogTemp, Warning, TEXT("MINUS drawsitance for lastRowZ"));
			lastRowZ = InitialChunkCoords.Y - DrawDistance;
			newRowZ = InitialChunkCoords.Y + DrawDistance;
		} else {
			// Get the last row by substracting the draw distance from the the initial Z
			UE_LOG(LogTemp, Warning, TEXT("PLUS drawsitance for lastRowZ"));
			lastRowZ = PlayerChunkCoords.Y + DrawDistance;
			newRowZ = PlayerChunkCoords.Y - DrawDistance;
		}

		int firstIndexChunkX = PlayerChunkCoords.X - DrawDistance + 1;
		int lastIndexChunkX = PlayerChunkCoords.X + DrawDistance + 1;

		UE_LOG(LogTemp, Warning, TEXT("Attempting to get: X=%d to X=%d - Z=%d"), firstIndexChunkX, lastIndexChunkX, lastRowZ);
		UE_LOG(LogTemp, Warning, TEXT("InitialChunkCoords: X=%d Z=%d "), InitialChunkCoords.X, InitialChunkCoords.Y);
		UE_LOG(LogTemp, Warning, TEXT("PlayerChunkCoords: X=%d Z=%d "), PlayerChunkCoords.X, PlayerChunkCoords.Y);


		// Loop and remove the entire row of chunks 
		for (int x = firstIndexChunkX; x < lastIndexChunkX; x++) {
			FIntPoint oldChunkCoords = FIntPoint(x, lastRowZ);
			AActor* currentChunk = SpawnedChunksMap.FindAndRemoveChecked(oldChunkCoords);
			currentChunk->Destroy();

			FIntPoint newChunkCoords = FIntPoint(x, newRowZ);
			FVector ChunkPosition = FVector(x * ChunkSize * UnrealScale, newRowZ * ChunkSize * UnrealScale, 0);
			AActor* SpawnedChunk = GetWorld()->SpawnActor<AActor>(Chunk, ChunkPosition, FRotator::ZeroRotator);

			if (SpawnedChunk) {
				SpawnedChunksMap.Add(newChunkCoords, SpawnedChunk);
			}
		}

		// Print SpawnedChunksMap keys after adding
		testCounter = 0;
		keysString = "SpawnedChunksMap Keys AFTER Z update: \n";
		for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {
			keysString += FString::Printf(TEXT("\tX=%d, Z=%d "), Pair.Key.X, Pair.Key.Y);
			testCounter++;

			if (testCounter == DrawDistance * 2) {
				keysString += TEXT("\n");
				testCounter = 0;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("%s"), *keysString);

	}

	 // Update the initial position for the next frame
	 playerInitialPosition = PlayerPosition;
}

void AChunkWorld::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

void AChunkWorld::spawnInitialWorld() {
	int spawnedChunks{ 0 };
	Time start = std::chrono::high_resolution_clock::now();

	for (int x = -DrawDistance; x < DrawDistance; x++) {
		for (int z = -DrawDistance; z < DrawDistance; z++) {
			FVector ChunkPosition = FVector(x * ChunkSize * UnrealScale, z * ChunkSize * UnrealScale, 0);
            AActor* SpawnedChunk = GetWorld()->SpawnActor<AActor>(Chunk, ChunkPosition, FRotator::ZeroRotator);
            if (SpawnedChunk) {
                SpawnedChunksMap.Add(FIntPoint(x, z), SpawnedChunk);
				spawnedChunks++;
            }
		}
	}

	Time end = std::chrono::high_resolution_clock::now();

	printExecutionTime(start, end, "Spawned entire terrain.");
	UE_LOG(LogTemp, Warning, TEXT("Chunks spawned: %d"), spawnedChunks);
}

FIntPoint AChunkWorld::GetChunkCoordinates(FVector Position) const {
	int32 ChunkX = FMath::FloorToInt(Position.X / (ChunkSize * UnrealScale));
	int32 ChunkY = FMath::FloorToInt(Position.Y / (ChunkSize * UnrealScale));
	return FIntPoint(ChunkX, ChunkY);
}

// Called when the game starts or when spawned
void AChunkWorld::BeginPlay() {
	Super::BeginPlay();

	spawnInitialWorld();
	
	// Set player's initial position
	playerInitialPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

	ADefaultPawn* PlayerPawn = Cast<ADefaultPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (PlayerPawn) {
		UFloatingPawnMovement* MovementComponent = Cast<UFloatingPawnMovement>(PlayerPawn->GetMovementComponent());
		if (MovementComponent) {
			MovementComponent->MaxSpeed = 10000.0f;
			UE_LOG(LogTemp, Warning, TEXT("Player speed set to %f"), MovementComponent->MaxSpeed);
		} else {
			UE_LOG(LogTemp, Error, TEXT("Player pawn does not have a movement component!"));
		}
	} else {
		UE_LOG(LogTemp, Error, TEXT("Player pawn is not a DefaultPawn!"));
	}
}

// Called every frame
void AChunkWorld::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	UpdateChunks();
}


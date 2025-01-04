// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "..\TerrainSettings\WorldTerrainSettings.h"
#include "..\ChunkData\ChunkLocationData.h"
#include "..\SingleChunk\BinaryChunk.h" 
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include <Kismet/GameplayStatics.h>
#include <set>

// Sets default values
AChunkWorld::AChunkWorld() : chunksLocationRunnable(nullptr), chunksLocationThread(nullptr), isLocationTaskRunning(false), isMeshTaskRunning(false){
	// Set this actor to call Tick() every frame.  Yosu can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	isInitialWorldGenerated = false;

	// Initializing Chunk with the BinaryChunk class
	Chunk = ABinaryChunk::StaticClass();

	// Initializing Tree with the Tree class
	Tree = ATree::StaticClass();
}

void AChunkWorld::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void AChunkWorld::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}

void AChunkWorld::SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings) {
	PerlinNoiseSettingsRef = InPerlinNoiseSettings;

	// Create all the instances of perlin noise and set their settings
	WTSR->SetPerlinNoiseSettings(InPerlinNoiseSettings);
}

void AChunkWorld::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

void AChunkWorld::spawnInitialWorld() {
	int spawnedChunks{ 0 };
	Time start = std::chrono::high_resolution_clock::now();

	// Add initial chunk position to spawn
	FVector ChunkPosition = FVector(0, 0, 0);
	FIntPoint ChunkWorldCoords = FIntPoint(0, 0);
	CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, ChunkWorldCoords));

	// Add points for tree radius 
	WTSR->AddPlayer2DTreeRadiusPoint(ChunkWorldCoords);

	// Add chunk positions to spawn by going in a spiral from origin position
	std::set<std::pair<int, int>> avoidPosition = { {0,0} };
	int currentSpiralRing = 1;
	int maxSpiralRings = WTSR->DrawDistance;

	while (currentSpiralRing <= maxSpiralRings) {
		for (int x = -currentSpiralRing; x < currentSpiralRing; x++) {
			for (int z = -currentSpiralRing; z < currentSpiralRing; z++) {
				/*for (int x = 0; x < 1; x++) {
					for (int z = 0; z < 1; z++) {*/
				std::pair<int, int> currentPair = { x, z };

				if (avoidPosition.find(currentPair) != avoidPosition.end()) {
					continue;
				}

				ChunkPosition = FVector(x * WTSR->chunkSize * WTSR->UnrealScale, z * WTSR->chunkSize * WTSR->UnrealScale, 0);
				ChunkWorldCoords = FIntPoint(x, z);
				CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, ChunkWorldCoords));

				// Add world coordinates for tree spawning if is inside the tree radius
				if (WTSR->isPointWithinTreeRadiusRange(ChunkWorldCoords)) {
					WTSR->AddPlayer2DTreeRadiusPoint(ChunkWorldCoords);
				}

				avoidPosition.insert(currentPair);
			}
		}
		currentSpiralRing++;
	}

	Time end = std::chrono::high_resolution_clock::now();

	printExecutionTime(start, end, "Spawned entire terrain.");
	UE_LOG(LogTemp, Warning, TEXT("Chunks spawned: %d"), spawnedChunks);
	UE_LOG(LogTemp, Warning, TEXT("SpawnedChunkMap size = %d"), WTSR->GetMapSize());
	UE_LOG(LogTemp, Warning, TEXT("DrawDistance = %d"), WTSR->DrawDistance);
}

void AChunkWorld::generateTreeMeshVariations() {
	Time start = std::chrono::high_resolution_clock::now();

	UTreeMeshGenerator* TreeMeshGenerator = NewObject<UTreeMeshGenerator>();
	TreeMeshGenerator->SetWorldTerrainSettings(WTSR);

	for (int treeIndex = 0; treeIndex < WTSR->TreeVariations; treeIndex++) {
		FVoxelObjectMeshData treeMeshData = TreeMeshGenerator->GetMeshTreeMeshData();
		WTSR->AddTreeMeshData(treeMeshData);
	}

	Time end = std::chrono::high_resolution_clock::now();
	printExecutionTime(start, end, std::format("Generated {} tree variations.", WTSR->TreeVariations).c_str());

}

void AChunkWorld::generateGrassMeshVariations() {
	Time start = std::chrono::high_resolution_clock::now();



	Time end = std::chrono::high_resolution_clock::now();
	printExecutionTime(start, end, std::format("Generated {} grass variations.", WTSR->GrassVariations).c_str());
}

void AChunkWorld::generateFlowerMeshVariations() {
	Time start = std::chrono::high_resolution_clock::now();




	Time end = std::chrono::high_resolution_clock::now();
	printExecutionTime(start, end, std::format("Generated {} flower variations.", WTSR->FlowerVariations).c_str());
}

// Perform any actions after generating the new chunks
void AChunkWorld::onNewTerrainGenerated() {
	// Spawn the tree TODO Continue from here 
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

void AChunkWorld::SpawnTrees(FVoxelObjectLocationData ChunkLocationData, FVector PlayerPosition) {
	// Spawn the chunk actor deferred
	ATree* SpawnedTreeActor = GetWorld()->SpawnActorDeferred<ATree>(Tree, FTransform(FRotator::ZeroRotator, ChunkLocationData.ObjectPosition), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (SpawnedTreeActor) {
		// Add references to BinaryChunk and pass the computed mesh data
		SpawnedTreeActor->SetWorldTerrainSettings(WTSR);
		SpawnedTreeActor->SetPerlinNoiseSettings(PNSR);
		SpawnedTreeActor->SetTreeMeshData(WTSR->GetRandomTreeMeshData());

		// Define the boundaries for the collision check
		float minX = PlayerPosition.X - WTSR->VegetationCollisionDistance;
		float maxX = PlayerPosition.X + WTSR->VegetationCollisionDistance;
		float minY = PlayerPosition.Y - WTSR->VegetationCollisionDistance;
		float maxY = PlayerPosition.Y + WTSR->VegetationCollisionDistance;

		// Check if the player is within the collision boundaries
		bool withinCollisionDistance = (ChunkLocationData.ObjectPosition.X >= minX && ChunkLocationData.ObjectPosition.X <= maxX) &&
			(ChunkLocationData.ObjectPosition.Y >= minY && ChunkLocationData.ObjectPosition.Y <= maxY);

		if (withinCollisionDistance) {
			 SpawnedTreeActor->SetTreeCollision(true);
		}

		// Finish spawning the chunk actor
		UGameplayStatics::FinishSpawningActor(SpawnedTreeActor, FTransform(FRotator::ZeroRotator, ChunkLocationData.ObjectPosition));

		// TODO Add the tree actor to a map so I can update the collision and remove it later on
		WTSR->AddSpawnedTrees(ChunkLocationData.ObjectWorldCoords, SpawnedTreeActor);

		//UE_LOG(LogTemp, Warning, TEXT("Spawned Tree!"));
	} else {
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Tree Actor!"));
	}
}



// Called when the game starts or when spawned
void AChunkWorld::BeginPlay() {
	Super::BeginPlay();

	spawnInitialWorld();

	generateTreeMeshVariations();
	generateGrassMeshVariations();
	generateFlowerMeshVariations();

	// Set player's initial position
	WTSR->updateInitialPlayerPosition(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation());

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
	
	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

	const FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	const FIntPoint InitialChunkCoords = GetChunkCoordinates(WTSR->getInitialPlayerPosition());

	const bool isPlayerMovingOnAxisX = PlayerChunkCoords.X != InitialChunkCoords.X; //   // TODO SET TO FALSE FOR TESTING ONLY
	const bool isPlayerMovingOnAxisZ = PlayerChunkCoords.Y != InitialChunkCoords.Y; //  // TODO SET TO FALSE FOR TESTING ONLY

	if (!isLocationTaskRunning && (isPlayerMovingOnAxisX || isPlayerMovingOnAxisZ)) {
		isLocationTaskRunning.AtomicSet(true);
		chunksLocationRunnable = new ChunksLocationRunnable(PlayerPosition, WTSR, CLDR);
		chunksLocationThread = FRunnableThread::Create(chunksLocationRunnable, TEXT("chunksLocationThread"), 0, TPri_Normal);
	}
	// TODO ADD TO A LIST CHUNKS THAT NEED THEIR COLLISION UPDATED IN THAT SEPARATE THREAD
	// TODO ADD TO A LIST TREES THAT NEED THEIR COLLISION UPDATED IN THAT SEPARATE THREAD

	// Clean up terrain thread if it's done computing
	if (chunksLocationRunnable && chunksLocationRunnable->IsTaskComplete()) {
		onNewTerrainGenerated();

		if (chunksLocationThread) {
			chunksLocationRunnable->Stop();
			chunksLocationThread->WaitForCompletion();
			delete chunksLocationThread;
			chunksLocationThread = nullptr;
		}

		if (chunksLocationRunnable) {
			delete chunksLocationRunnable;
			chunksLocationRunnable = nullptr;
		}

		isLocationTaskRunning.AtomicSet(false);
	}

	// Create mesh for chunk position if there is a position waiting to be processed
	if (!isMeshTaskRunning) {
		FVoxelObjectLocationData chunkToSpawnPosition;
		const bool doesSpawnPositionExist = CLDR->getChunkToSpawnPosition(chunkToSpawnPosition);

		if (doesSpawnPositionExist) {
			// Calculate the chunk mesh data in a separate thread
			isMeshTaskRunning.AtomicSet(true);
			chunkMeshDataRunnable = new ChunkMeshDataRunnable(chunkToSpawnPosition, WTSR, CLDR);
			chunkMeshDataThread = FRunnableThread::Create(chunkMeshDataRunnable, TEXT("chunkMeshDataThread"), 0, TPri_Normal);
		}
	}

	// Cleanup mesh thread if it's done computing
	if (chunkMeshDataRunnable && chunkMeshDataRunnable->IsTaskComplete()) {
		onNewTerrainGenerated();

		if (chunkMeshDataThread) {
			chunkMeshDataRunnable->Stop();
			chunkMeshDataThread->WaitForCompletion();
			delete chunkMeshDataThread;
			chunkMeshDataThread = nullptr;
		}

		if (chunkMeshDataRunnable) {
			delete chunkMeshDataRunnable;
			chunkMeshDataRunnable = nullptr;
		}

		isMeshTaskRunning.AtomicSet(false);
	}

	const TArray<FIntPoint> tempTreeRadiusPoints = WTSR->GetPlayer2DTreeRadiusPoints();

	// Add trees to the chunk
	if (CLDR->isTreeWaitingToBeSpawned(tempTreeRadiusPoints)) {
		// Get all tree positions that are within the tree spawn radius around the player
		TArray<FVoxelObjectLocationData> treeSpawnPositions = CLDR->getTreeSpawnPositions(tempTreeRadiusPoints);

		// Spawn all the trees that are in the returned spawn radius array
		for (const FVoxelObjectLocationData& treePosition : treeSpawnPositions) {
			SpawnTrees(treePosition, PlayerPosition);
			WTSR->TreeCount++;

			// Print the tree count every 50
			if (WTSR->TreeCount % 50 == 0) {
				UE_LOG(LogTemp, Log, TEXT("Tree count: %d"), WTSR->TreeCount);
			}
		}
	}

	if (CLDR->isGrassWaitingToBeSpawned()) {

	}

	if (CLDR->isFlowerWaitingToBeSpawned()) {

	}

	// Spawn chunk if there is a calculated mesh data waiting
	if (CLDR->isMeshWaitingToBeSpawned()) {
		
		// Get the location data and the computed mesh data for the chunk
		FVoxelObjectLocationData waitingMeshLocationData;
		FVoxelObjectMeshData waitingMeshData;
		CLDR->getComputedMeshDataAndLocationData(waitingMeshLocationData, waitingMeshData);

		Time start = std::chrono::high_resolution_clock::now();

		// Spawn the chunk actor deferred
		ABinaryChunk* SpawnedChunkActor = GetWorld()->SpawnActorDeferred<ABinaryChunk>(Chunk, FTransform(FRotator::ZeroRotator, waitingMeshLocationData.ObjectPosition), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (SpawnedChunkActor) {
				// Add references to BinaryChunk and pass the computed mesh data
				SpawnedChunkActor->SetWorldTerrainSettings(WTSR);
				SpawnedChunkActor->SetPerlinNoiseSettings(PNSR);
				SpawnedChunkActor->SetComputedMeshData(waitingMeshData);

				// Define the boundaries for the collision check
				float minX = PlayerPosition.X - WTSR->CollisionDistance;
				float maxX = PlayerPosition.X + WTSR->CollisionDistance;
				float minY = PlayerPosition.Y - WTSR->CollisionDistance;
				float maxY = PlayerPosition.Y + WTSR->CollisionDistance;

				// Check if the player is within the collision boundaries
				bool withinCollisionDistance = (waitingMeshLocationData.ObjectPosition.X >= minX && waitingMeshLocationData.ObjectPosition.X <= maxX) &&
					(waitingMeshLocationData.ObjectPosition.Y >= minY && waitingMeshLocationData.ObjectPosition.Y <= maxY);

				if (withinCollisionDistance) {
					SpawnedChunkActor->SetChunkCollision(true);
				}

				// Finish spawning the chunk actor
				UGameplayStatics::FinishSpawningActor(SpawnedChunkActor, FTransform(FRotator::ZeroRotator, waitingMeshLocationData.ObjectPosition));

				WTSR->AddChunkToMap(waitingMeshLocationData.ObjectWorldCoords, SpawnedChunkActor);
		} else {
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn Chunk Actor!"));
		}

		Time end = std::chrono::high_resolution_clock::now();
		calculateAverageChunkSpawnTime(start, end);
	}

	// Destroy a chunk and remove it from the map if there is a destroy position in queue
	FIntPoint chunkToDestroyPosition{};
	bool doesDestroyPositionExist = CLDR->getChunkToDestroyPosition(chunkToDestroyPosition);

	// I need to check if the destroy position exists in the map, otherwise I need to push it back 
	if (doesDestroyPositionExist) {
		AActor* chunkToRemove = WTSR->GetAndRemoveChunkFromMap(chunkToDestroyPosition);
		if (chunkToRemove) {
			chunkToRemove->Destroy();
		} else {
			// Add the chunk position back because the chunk is not yet spawned
			CLDR->addChunksToDestroyPosition(chunkToDestroyPosition); // TODO Optimize this, as it keeps getting removed and added back
		}
	}

	FIntPoint treeToDestroyPosition{};
	bool doesTreeDestroyPositionExist = CLDR->getTreeToDestroyPosition(treeToDestroyPosition);

	// I need to check if the destroy position exists in the map, otherwise I need to push it back 
	if (doesTreeDestroyPositionExist) {
		TArray<ATree*> treesToRemove = WTSR->GetAndRemoveTreeFromMap(treeToDestroyPosition);

		for (ATree* treeToRemove : treesToRemove) {
			if (treeToRemove) {
				treeToRemove->Destroy();
				WTSR->TreeCount--;
			} else {
				// Add the tree position back because the tree is not yet spawned
				WTSR->AddSpawnedTrees(treeToDestroyPosition, treeToRemove); // TODO THIS IS A MAYBE. I SHOULD STILL FOLLOW HOW THE CHUNKS MAP IS DONE
				CLDR->addTreeToDestroyPosition(treeToDestroyPosition); // TODO Optimize this, as it keeps getting removed and added back
			}
		}

	}

	
	// TODO ADD AND REMOVE NEW TREES AS THE PLAYER MOVES IN THE WORLD 

	// Enabling and disabling collision for chunks and trees
	ATree* removeCollisionTree = WTSR->GetTreeToRemoveCollision();
	ATree* enableCollisionTree = WTSR->GetTreeToEnableCollision(); 
	
	ABinaryChunk* removeCollisionChunk = WTSR->GetChunkToRemoveCollision();
	ABinaryChunk* enableCollisionChunk = WTSR->GetChunkToEnableCollision();

	if (removeCollisionTree) {
		removeCollisionTree->UpdateCollision(false);
	}

	if (enableCollisionTree) {
		enableCollisionTree->UpdateCollision(true);
	}

	if (removeCollisionChunk) {
		removeCollisionChunk->UpdateCollision(false);
	}

	if (enableCollisionChunk) {
		enableCollisionChunk->UpdateCollision(true);
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
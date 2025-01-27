// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "..\TerrainSettings\WorldTerrainSettings.h"
#include "..\ChunkData\ChunkLocationData.h"
#include "..\SingleChunk\BinaryChunk.h" 
#include "..\..\NPC\BasicNPC\BasicNPC.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include <Kismet/GameplayStatics.h>

#include "ProceduralMeshComponent.h" // MAYBE

#include <set>

// Sets default values
AChunkWorld::AChunkWorld() : chunksLocationRunnable(nullptr), chunksLocationThread(nullptr), isLocationTaskRunning(false), isMeshTaskRunning(false) {
	// Set this actor to call Tick() every frame.  Yosu can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Locking the tick at 60fps
	// PrimaryActorTick.TickInterval = 1.0f / 60.0f;

	isInitialWorldGenerated = false;

	// Initializing Chunk with the BinaryChunk class
	Chunk = ABinaryChunk::StaticClass();

	// Initializing Tree with the Tree class
	Tree = ATree::StaticClass();

	NPC = ABasicNPC::StaticClass();

	// Initialize thread pool for the NPC pathfinding
	const int PathfindingThreads = 3;
	PathfindingManager = new PathfindingThreadManager(WTSR, CLDR, PathfindingThreads);
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
	FIntPoint PlayerStartCoords = FIntPoint(0, 0);
	FVector ChunkPosition = FVector(0, 0, 0);
	FIntPoint ChunkWorldCoords = FIntPoint(0, 0);
	CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, ChunkWorldCoords));

	// Add chunk positions to spawn by going in a spiral from origin position
	std::set<std::pair<int, int>> avoidPosition = { {0,0} };
	int currentSpiralRing = 1;
	int maxSpiralRings = WTSR->DrawDistance;

	while (currentSpiralRing <= maxSpiralRings) {
		for (int x = -currentSpiralRing; x < currentSpiralRing; x++) {
			FString rowString; // TESTING
			for (int z = -currentSpiralRing; z < currentSpiralRing; z++) {
				std::pair<int, int> currentPair = { x, z };

				if (avoidPosition.find(currentPair) != avoidPosition.end()) {
					continue;
				}

				ChunkPosition = FVector(x * WTSR->chunkSize * WTSR->UnrealScale, z * WTSR->chunkSize * WTSR->UnrealScale, 0);
				ChunkWorldCoords = FIntPoint(x, z);

				CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, ChunkWorldCoords));

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
		const FColor trunkColor = WTSR->TreeTrunkColorArray[treeIndex % WTSR->TreeTrunkColorArray.Num()];
		const FColor crownColor = WTSR->TreeCrownColorArray[treeIndex % WTSR->TreeCrownColorArray.Num()];

		FVoxelObjectMeshData treeMeshData = TreeMeshGenerator->GetTreeMeshData(trunkColor, crownColor);
		WTSR->AddTreeMeshData(treeMeshData);
	}

	Time end = std::chrono::high_resolution_clock::now();
	printExecutionTime(start, end, std::format("Generated {} tree variations.", WTSR->TreeVariations).c_str());

}

void AChunkWorld::generateGrassMeshVariations() {
	Time start = std::chrono::high_resolution_clock::now();

	UGrassMeshGenerator* GrassMeshGenerator = NewObject<UGrassMeshGenerator>();
	GrassMeshGenerator->SetWorldTerrainSettings(WTSR);

	for (int grassIndex = 0; grassIndex < WTSR->GrassVariations; grassIndex++) {
		const FColor grassColor = WTSR->GrassBladesColorArray[grassIndex % WTSR->GrassBladesColorArray.Num()];
		FVoxelObjectMeshData grassMeshData = GrassMeshGenerator->GetGrassMeshData(grassColor);
		WTSR->AddGrassMeshData(grassMeshData);
	}

	Time end = std::chrono::high_resolution_clock::now();
	printExecutionTime(start, end, std::format("Generated {} grass variations.", WTSR->GrassVariations).c_str());
}

void AChunkWorld::generateFlowerMeshVariations() {
	Time start = std::chrono::high_resolution_clock::now();

	UFlowerMeshGenerator* FlowerMeshGenerator = NewObject<UFlowerMeshGenerator>();
	FlowerMeshGenerator->SetWorldTerrainSettings(WTSR);

	for (int flowerIndex = 0; flowerIndex < WTSR->FlowerVariations; flowerIndex++) {
		const FColor stemColor = WTSR->FlowerStemColorArray[flowerIndex % WTSR->FlowerStemColorArray.Num()];
		const FColor petalColor = WTSR->FlowerPetalColorArray[flowerIndex % WTSR->FlowerPetalColorArray.Num()];

		FVoxelObjectMeshData flowerMeshData = FlowerMeshGenerator->GetFlowerMeshData(stemColor, petalColor);
		WTSR->AddFlowerMeshData(flowerMeshData);
	}

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
		SpawnedTreeActor->SetTreeWorldLocation(ChunkLocationData.ObjectWorldCoords);

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
	} else {
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Tree Actor!"));
	}
}

void AChunkWorld::SpawnGrass(FVoxelObjectLocationData ChunkLocationData, FVector PlayerPosition) {
	UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(this);
	Mesh->RegisterComponent();
	Mesh->SetCastShadow(false);

	FVoxelObjectMeshData* MeshData = WTSR->GetRandomGrassMeshData();
	Mesh->CreateMeshSection(0, MeshData->Vertices, MeshData->Triangles, MeshData->Normals, MeshData->UV0, MeshData->Colors, TArray<FProcMeshTangent>(), false);

	// Set up simplified collision
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	// Load and apply basic material to the mesh
	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/VoxelBasicMaterial.VoxelBasicMaterial"));

	if (Material) {
		Mesh->SetMaterial(0, Material);
	}

	Mesh->SetWorldLocation(ChunkLocationData.ObjectPosition);
	Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// Adding the tree object to a map so I can remove it later on
	WTSR->AddSpawnedGrass(ChunkLocationData.ObjectWorldCoords, Mesh);
}

// TODO Combine spawngrass and spawnflower into one method
void AChunkWorld::SpawnFlower(FVoxelObjectLocationData ChunkLocationData, FVector PlayerPosition) {
	UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(this);
	Mesh->RegisterComponent();
	Mesh->SetCastShadow(false);

	FVoxelObjectMeshData* MeshData = WTSR->GetRandomFlowerMeshData();
	Mesh->CreateMeshSection(0, MeshData->Vertices, MeshData->Triangles, MeshData->Normals, MeshData->UV0, MeshData->Colors, TArray<FProcMeshTangent>(), false);

	// Set up simplified collision
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	// Load and apply basic material to the mesh
	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/VoxelBasicMaterial.VoxelBasicMaterial"));

	if (Material) {
		Mesh->SetMaterial(0, Material);
	}

	Mesh->SetWorldLocation(ChunkLocationData.ObjectPosition);
	Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// Adding the tree object to a map so I can remove it later on
	WTSR->AddSpawnedFlower(ChunkLocationData.ObjectWorldCoords, Mesh);
}

void AChunkWorld::SpawnNPC(FVoxelObjectLocationData ChunkLocationData, FVector PlayerPosition) {
	// Spawn the NPC actor deferred
	ABasicNPC* SpawnedNPCActor = GetWorld()->SpawnActorDeferred<ABasicNPC>(NPC, FTransform(FRotator::ZeroRotator, ChunkLocationData.ObjectPosition), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (SpawnedNPCActor) {
		// Add references to BinaryChunk and pass the computed mesh data
		/*SpawnedTreeActor->SetWorldTerrainSettings(WTSR);
		SpawnedTreeActor->SetPerlinNoiseSettings(PNSR);
		SpawnedTreeActor->SetTreeMeshData(WTSR->GetRandomTreeMeshData());
		SpawnedTreeActor->SetTreeWorldLocation(ChunkLocationData.ObjectWorldCoords);*/
		SpawnedNPCActor->SetWorldTerrainSettings(WTSR);
		SpawnedNPCActor->SetNPCWorldLocation(ChunkLocationData.ObjectWorldCoords);

		// Define the boundaries for the collision check
		//float minX = PlayerPosition.X - WTSR->VegetationCollisionDistance;
		//float maxX = PlayerPosition.X + WTSR->VegetationCollisionDistance;
		//float minY = PlayerPosition.Y - WTSR->VegetationCollisionDistance;
		//float maxY = PlayerPosition.Y + WTSR->VegetationCollisionDistance;

		//// Check if the player is within the collision boundaries
		//bool withinCollisionDistance = (ChunkLocationData.ObjectPosition.X >= minX && ChunkLocationData.ObjectPosition.X <= maxX) &&
		//	(ChunkLocationData.ObjectPosition.Y >= minY && ChunkLocationData.ObjectPosition.Y <= maxY);

		//if (withinCollisionDistance) {
		//	SpawnedTreeActor->SetTreeCollision(true);
		//}

		// Finish spawning the chunk actor
		UGameplayStatics::FinishSpawningActor(SpawnedNPCActor, FTransform(FRotator::ZeroRotator, ChunkLocationData.ObjectPosition));

		// TODO Add the tree actor to a map so I can update the collision and remove it later on
		// WTSR->AddSpawnedTrees(ChunkLocationData.ObjectWorldCoords, SpawnedTreeActor);
	} else {
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn NPC Actor!"));
	}

}

// Remove the vegetation (tree, grass, flowers) spawn points, and add the actor pointers
// to a local cache, to be removed across multiple frames in Tick().
void AChunkWorld::RemoveVegetationSpawnPointsAndActors(const FIntPoint& destroyPosition) {
	// Remove remaining trees to spawn position at current chunk destroyed
	CLDR->RemoveTreeSpawnPosition(destroyPosition);

	// Remove trees to spawn in the ChunkWorld cache
	TreePositionsToSpawn.RemoveAll([&](const FVoxelObjectLocationData& Item) {
		return Item.ObjectWorldCoords == destroyPosition;
		});

	// Add spawned trees to a remove list to remove over multiple frames
	TArray<ATree*> treesToRemove = WTSR->GetAndRemoveTreeFromMap(destroyPosition);
	TreeActorsToRemove.Append(treesToRemove);

	// Remove remaining grass to spawn position at current chunk destroyed
	CLDR->RemoveGrassSpawnPosition(destroyPosition);

	// Remove grass to spawn in the ChunkWorld cache
	GrassPositionsToSpawn.RemoveAll([&](const FVoxelObjectLocationData& Item) {
		return Item.ObjectWorldCoords == destroyPosition;
		});

	// Add spawned grass to a remove list to remove over multiple frames
	TArray<UProceduralMeshComponent*> grassToRemove = WTSR->GetAndRemoveGrassFromMap(destroyPosition);
	GrassActorsToRemove.Append(grassToRemove);

	// Remove remaining flower to spawn position at current chunk destroyed
	CLDR->RemoveFlowerSpawnPosition(destroyPosition);

	// Remove flower to spawn in the ChunkWorld cache
	FlowerPositionsToSpawn.RemoveAll([&](const FVoxelObjectLocationData& Item) {
		return Item.ObjectWorldCoords == destroyPosition;
		});

	// Add spawned flower to a remove list to remove over multiple frames
	TArray<UProceduralMeshComponent*> flowerToRemove = WTSR->GetAndRemoveFlowerFromMap(destroyPosition);
	FlowerActorsToRemove.Append(flowerToRemove);
}

void AChunkWorld::DestroyTreeActors() {
	// Remove tree actors
	int removedTreeCounter = 0;
	for (int32 treeIndex = 0; treeIndex < TreeActorsToRemove.Num();) {
		if (removedTreeCounter >= treesToRemovePerFrame) {
			break;
		}

		ATree* treeToRemove = TreeActorsToRemove[treeIndex];
		if (treeToRemove) {
			treeToRemove->Destroy();
			WTSR->TreeCount--;
		} else {
			CLDR->AddUnspawnedTreeToDestroy(treeToRemove);
		}

		TreeActorsToRemove.RemoveAt(treeIndex);
		removedTreeCounter++;
	}

	// Check for any previously unspawned trees that could be now destroyed 
	ATree* unspawnedTree = nullptr;
	CLDR->GetUnspawnedTreeToDestroy(unspawnedTree);
	if (unspawnedTree) {
		// Destroy if it's valid and part of the world
		if (IsValid(unspawnedTree) && unspawnedTree->IsActorInitialized()) {
			unspawnedTree->Destroy();
		} else {
			// Add back to the list and wait for the tree to be spawned.
			CLDR->AddUnspawnedTreeToDestroy(unspawnedTree);
		}
	}
}

void AChunkWorld::DestroyGrassActors() {
	// Remove grass actors
	int removedGrassCounter = 0;
	for (int32 grassIndex = 0; grassIndex < GrassActorsToRemove.Num();) {
		if (removedGrassCounter >= grassToRemovePerFrame) {
			break;
		}

		UProceduralMeshComponent* grassToRemove = GrassActorsToRemove[grassIndex];
		if (grassToRemove) {
			grassToRemove->UnregisterComponent();
			grassToRemove->DestroyComponent();
			//grassToRemove->Destroy();
			WTSR->GrassCount--;
		} else {
			CLDR->AddUnspawnedGrassToDestroy(grassToRemove);
		}

		GrassActorsToRemove.RemoveAt(grassIndex);
		removedGrassCounter++;
	}

	// Check for any previously unspawned grass that could be now destroyed 
	UProceduralMeshComponent* unspawnedGrass = nullptr;
	CLDR->AddUnspawnedGrassToDestroy(unspawnedGrass);
	if (unspawnedGrass) {
		// Destroy if it's valid and part of the world
		if (IsValid(unspawnedGrass)) {
			unspawnedGrass->UnregisterComponent();
			unspawnedGrass->DestroyComponent();
		} else {
			// Add back to the list and wait for the grass to be spawned.
			CLDR->AddUnspawnedGrassToDestroy(unspawnedGrass);
		}
	}
}

void AChunkWorld::DestroyFlowerActors() {
	// Remove flower actors
	int removedFlowerCounter = 0;
	for (int32 flowerIndex = 0; flowerIndex < FlowerActorsToRemove.Num();) {
		if (removedFlowerCounter >= flowerToRemovePerFrame) {
			break;
		}

		UProceduralMeshComponent* flowerToRemove = FlowerActorsToRemove[flowerIndex];
		if (flowerToRemove) {
			flowerToRemove->UnregisterComponent();
			flowerToRemove->DestroyComponent();
			WTSR->FlowerCount--;
		} else {
			CLDR->AddUnspawnedFlowerToDestroy(flowerToRemove);
		}

		FlowerActorsToRemove.RemoveAt(flowerIndex);
		removedFlowerCounter++;
	}

	// Check for any previously unspawned flower that could be now destroyed 
	UProceduralMeshComponent* unspawnedFlower = nullptr;
	CLDR->AddUnspawnedFlowerToDestroy(unspawnedFlower);
	if (unspawnedFlower) {
		// Destroy if it's valid and part of the world
		if (IsValid(unspawnedFlower)) {
			unspawnedFlower->UnregisterComponent();
			unspawnedFlower->DestroyComponent();
		} else {
			// Add back to the list and wait for the flower to be spawned.
			CLDR->AddUnspawnedFlowerToDestroy(unspawnedFlower);
		}
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

void AChunkWorld::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	// Cleanup 
	if (PathfindingManager) {
		PathfindingManager->ShutDownThreadPool();
		delete PathfindingManager;
		PathfindingManager = nullptr;
	}
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

		// Update the noise?
		SetPerlinNoiseSettings(PerlinNoiseSettingsRef);

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

	// Attempt pathfinding 
	FVector startLocation = FVector(0, 0, 0);
	FVector endLocation = FVector(100, 100, 0);
	PathfindingManager->AddPathfindingTask(startLocation, endLocation);

	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

	const FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	const FIntPoint InitialChunkCoords = GetChunkCoordinates(WTSR->getInitialPlayerPosition());

	const bool isPlayerMovingOnAxisX = PlayerChunkCoords.X != InitialChunkCoords.X; // TODO SET TO FALSE FOR TESTING ONLY
	const bool isPlayerMovingOnAxisZ = PlayerChunkCoords.Y != InitialChunkCoords.Y; // TODO SET TO FALSE FOR TESTING ONLY

	if (!isLocationTaskRunning && (isPlayerMovingOnAxisX || isPlayerMovingOnAxisZ)) {
		isLocationTaskRunning.AtomicSet(true);
		chunksLocationRunnable = new ChunksLocationRunnable(PlayerPosition, WTSR, CLDR);
		chunksLocationThread = FRunnableThread::Create(chunksLocationRunnable, TEXT("chunksLocationThread"), 0, TPri_Normal);
	}

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
			chunkMeshDataRunnable = new ChunkMeshDataRunnable(chunkToSpawnPosition, WTSR, CLDR, PNSR);
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

	// Append tree positions waiting to be spawned
	TArray<FVoxelObjectLocationData> treeSpawnPositions = CLDR->getTreeSpawnPositions();
	TreePositionsToSpawn.Append(treeSpawnPositions);

	// Spawn a few trees in the current frame
	int spawnedTreeCounter = 0;
	for (int32 positionIndex = 0; positionIndex < TreePositionsToSpawn.Num();) {
		if (spawnedTreeCounter >= treesToSpawnPerFrame) {
			break;
		}

		SpawnTrees(TreePositionsToSpawn[positionIndex], PlayerPosition);
		WTSR->TreeCount++;

		// Print the tree count every 50
		if (WTSR->TreeCount % 1000 == 0) {
			UE_LOG(LogTemp, Log, TEXT("Tree count: %d"), WTSR->TreeCount);
		}

		TreePositionsToSpawn.RemoveAt(positionIndex);
		spawnedTreeCounter++;
	}

	// Append grass positions waiting to be spawned
	TArray<FVoxelObjectLocationData> grassSpawnPositions = CLDR->getGrassSpawnPosition();
	GrassPositionsToSpawn.Append(grassSpawnPositions);

	// Spawn a few trees in the current frame
	int spawnedGrassCounter = 0;
	for (int32 positionIndex = 0; positionIndex < GrassPositionsToSpawn.Num();) {
		if (spawnedGrassCounter >= grassToSpawnPerFrame) {
			break;
		}

		SpawnGrass(GrassPositionsToSpawn[positionIndex], PlayerPosition);
		WTSR->GrassCount++;

		// Print the tree count every 50
		/*if (WTSR->GrassCount % 1000 == 0) {
			UE_LOG(LogTemp, Log, TEXT("Grass count: %d"), WTSR->GrassCount);
		}*/

		GrassPositionsToSpawn.RemoveAt(positionIndex);
		spawnedGrassCounter++;
	}

	// Append flower positions waiting to be spawned
	TArray<FVoxelObjectLocationData> flowerSpawnPositions = CLDR->getFlowerSpawnPosition();
	FlowerPositionsToSpawn.Append(flowerSpawnPositions);

	// Spawn a few flowers in the current frame
	int spawnedFlowerCounter = 0;
	for (int32 positionIndex = 0; positionIndex < FlowerPositionsToSpawn.Num();) {
		if (spawnedFlowerCounter >= flowerToSpawnPerFrame) {
			break;
		}

		SpawnFlower(FlowerPositionsToSpawn[positionIndex], PlayerPosition);
		WTSR->FlowerCount++;

		// Print the tree count every 50
		/*if (WTSR->FlowerCount % 50 == 0) {
			UE_LOG(LogTemp, Log, TEXT("Flower count: %d"), WTSR->FlowerCount);
		}*/

		FlowerPositionsToSpawn.RemoveAt(positionIndex);
		spawnedFlowerCounter++;
	}

	// Append NPC positions waiting to be spawned
	TArray<FVoxelObjectLocationData> NPCSpawnPositions = CLDR->getNPCSpawnPosition();
	NPCPositionsToSpawn.Append(NPCSpawnPositions);

	// Spawn a few flowers in the current frame
	int spawnedNPCCounter = 0;
	for (int32 positionIndex = 0; positionIndex < NPCPositionsToSpawn.Num();) {
		if (spawnedNPCCounter >= npcToSpawnPerFrame) {
			break;
		}

		// SpawnNPC(NPCPositionsToSpawn[positionIndex], PlayerPosition);
		WTSR->NPCCount++;

		// Print the tree count every 50
		/*if (WTSR->NPCCount % 50 == 0) {
			UE_LOG(LogTemp, Log, TEXT("NPC count: %d"), WTSR->NPCCount);
		}*/

		NPCPositionsToSpawn.RemoveAt(positionIndex);
		spawnedNPCCounter++;
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
			SpawnedChunkActor->SetChunkLocation(waitingMeshLocationData.ObjectWorldCoords);

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

			// Remove remaining vegetation spawn points for the destroyed chunk location
			// and add the aactor pointers to a local cache to be removed across multiple frames
			RemoveVegetationSpawnPointsAndActors(chunkToDestroyPosition);

			// Remove the voxel surface points from the pathfinding map
			CLDR->RemoveSurfaceVoxelPointsForChunk(chunkToDestroyPosition);
		} else {
			// Add the chunk position back because the chunk is not yet spawned
			CLDR->addChunksToDestroyPosition(chunkToDestroyPosition); // TODO Optimize this, as it keeps getting removed and added back. I should use a TMap instead and remove the entry of that object instead, preventing it from spawning in the first place.
		}
	}

	// Destroy a few vegetation aactors
	DestroyTreeActors();
	DestroyGrassActors();
	DestroyFlowerActors();
	
	// Enabling and disabling collision for chunks and trees
	ATree* removeCollisionTree = WTSR->GetTreeToRemoveCollision();
	ATree* enableCollisionTree = WTSR->GetTreeToEnableCollision();

	ABinaryChunk* removeCollisionChunk = WTSR->GetChunkToRemoveCollision();
	ABinaryChunk* enableCollisionChunk = WTSR->GetChunkToEnableCollision();

	if (IsValid(removeCollisionTree) && removeCollisionTree->IsActorInitialized()) {
		removeCollisionTree->UpdateCollision(false);
	}

	if (IsValid(enableCollisionTree) && enableCollisionTree->IsActorInitialized()) {
		enableCollisionTree->UpdateCollision(true);
	}

	if (IsValid(removeCollisionChunk) && removeCollisionChunk->IsActorInitialized()) {
		removeCollisionChunk->UpdateCollision(false);
	}

	if (IsValid(enableCollisionChunk) && enableCollisionChunk->IsActorInitialized()) {
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
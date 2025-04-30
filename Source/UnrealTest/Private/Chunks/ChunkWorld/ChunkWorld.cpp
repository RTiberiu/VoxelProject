// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "..\TerrainSettings\WorldTerrainSettings.h"
#include "..\ChunkData\ChunkLocationData.h"
#include "..\SingleChunk\BinaryChunk.h" 
#include "..\..\NPC\BasicNPC\BasicNPC.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "..\..\NPC\SettingsNPC\RelationshipSettingsNPC.h"
#include <Kismet/GameplayStatics.h>

#include "ProceduralMeshComponent.h"

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

void AChunkWorld::SetAnimationSettingsNpc(UAnimationSettingsNPC* InAnimationSettingsRef) {
	AnimationSettingsRef = InAnimationSettingsRef;
}

void AChunkWorld::SetStatsVoxelsMeshNPC(UStatsVoxelsMeshNPC* InStatsVoxelsMeshNPC) {
	StatsVoxelsMeshNPCRef = InStatsVoxelsMeshNPC;
}

void AChunkWorld::InitializePathfindingManager() {
	// Initialize thread pool for the NPC pathfinding
	const int PathfindingThreads = 3;
	PathfindingManager = new PathfindingThreadManager(WTSR, CLDR, PathfindingThreads);
}

void AChunkWorld::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %d seconds, %d milliseconds to execute."), *FString(functionName),
		static_cast<int>((duration.count() / 1000)) % 60,
		static_cast<int>(fmod(duration.count(), 1000)));
}

void AChunkWorld::spawnInitialWorld() {
	int spawnedChunks{ 0 };

	// Add initial chunk position to spawn
	FIntPoint PlayerStartCoords = FIntPoint(0, 0);
	FVector ChunkPosition = FVector(0, 0, 0);
	FIntPoint ChunkWorldCoords = FIntPoint(0, 0);
	CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, ChunkWorldCoords));
	CLDR->AddVegetationChunkSpawnPosition(ChunkWorldCoords);
	CLDR->AddTreeChunkSpawnPosition(ChunkWorldCoords);
	CLDR->AddNpcChunkSpawnPosition(ChunkWorldCoords);

	// Add chunk positions to spawn by going in a spiral from origin position
	std::set<std::pair<int, int>> avoidPosition = { {0,0} };
	int currentSpiralRing = 1;
	int maxSpiralRings = WTSR->DrawDistance;
	int vegetationMax = WTSR->VegetationDrawDistance;
	int treeMax = WTSR->TreeDrawDistance;
	int npcMax = WTSR->NpcDrawDistance;

	while (currentSpiralRing <= maxSpiralRings) {
		for (int x = -currentSpiralRing; x < currentSpiralRing; x++) {
			for (int z = -currentSpiralRing; z < currentSpiralRing; z++) {
				std::pair<int, int> currentPair = { x, z };

				if (avoidPosition.find(currentPair) != avoidPosition.end()) {
					continue;
				}

				ChunkPosition = FVector(x * WTSR->chunkSize * WTSR->UnrealScale, z * WTSR->chunkSize * WTSR->UnrealScale, 0);
				ChunkWorldCoords = FIntPoint(x, z);

				CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, ChunkWorldCoords));

				int ringDistance = FMath::Max(FMath::Abs(x), FMath::Abs(z));
				if (ringDistance < vegetationMax) {
					CLDR->AddVegetationChunkSpawnPosition(ChunkWorldCoords);
				}

				if (ringDistance < treeMax) {
					CLDR->AddTreeChunkSpawnPosition(ChunkWorldCoords);
				}

				if (ringDistance < npcMax) {
					CLDR->AddNpcChunkSpawnPosition(ChunkWorldCoords);
				}

				avoidPosition.insert(currentPair);
			}
		}

		currentSpiralRing++;
	}
}

void AChunkWorld::UseTestingConfigurations(ConfigToRun configToRun) {
	UsingTestConfiguration = true;

	if (SpawnedConfigOnce) {
		return;
	}

	TestConfigParameters ConfigData;

	switch (configToRun) {
	case NotificationAttackNpc:
		ConfigData = TestingConfig::GetNotificationAttackNpcTest();
		break;

	case NotificationAttackFoodSource:
		ConfigData = TestingConfig::GetNotificationAttackFoodTest();
		break;

	default:
		return;
	}

	// Spawn all NPCs
	for (int Index = WTSR->NPCCount; Index < ConfigData.NpcPositions.Num(); Index++) {
		SpawnNPC(ConfigData.NpcPositions[Index]);
		WTSR->NPCCount++;
	}

	// Spawn all Grass
	if (WTSR->GrassCount == 0) {
		for (int Index = WTSR->GrassCount; Index < ConfigData.Grass.Num(); Index++) {
			SpawnGrass(ConfigData.Grass[Index]);
			WTSR->GrassCount++;
		}
	}

	// Spawn all Flowers
	if (WTSR->FlowerCount == 0) {
		for (int Index = WTSR->FlowerCount; Index < ConfigData.Flowers.Num(); Index++) {
			SpawnFlower(ConfigData.Flowers[Index]);
			WTSR->FlowerCount++;
		}
	}

	SpawnedConfigOnce = true;
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

void AChunkWorld::SpawnTrees(FVoxelObjectLocationData LocationData, FVector PlayerPosition) {
	// Spawn the chunk actor deferred
	ATree* SpawnedTreeActor = GetWorld()->SpawnActorDeferred<ATree>(Tree, FTransform(FRotator::ZeroRotator, LocationData.ObjectPosition), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (SpawnedTreeActor) {
		// Add references to BinaryChunk and pass the computed mesh data
		SpawnedTreeActor->SetWorldTerrainSettings(WTSR);
		SpawnedTreeActor->SetTreeMeshData(WTSR->GetRandomTreeMeshData());
		SpawnedTreeActor->SetTreeWorldLocation(LocationData.ObjectWorldCoords);
		SpawnedTreeActor->SetTreeChunkRelativeLocation(LocationData.ObjectPosition);

		// Define the boundaries for the collision check
		float minX = PlayerPosition.X - WTSR->VegetationCollisionDistance;
		float maxX = PlayerPosition.X + WTSR->VegetationCollisionDistance;
		float minY = PlayerPosition.Y - WTSR->VegetationCollisionDistance;
		float maxY = PlayerPosition.Y + WTSR->VegetationCollisionDistance;

		// Check if the player is within the collision boundaries
		bool withinCollisionDistance = (LocationData.ObjectPosition.X >= minX && LocationData.ObjectPosition.X <= maxX) &&
			(LocationData.ObjectPosition.Y >= minY && LocationData.ObjectPosition.Y <= maxY);

		if (withinCollisionDistance) {
			SpawnedTreeActor->SetTreeCollision(true);
		}

		// Finish spawning the chunk actor
		UGameplayStatics::FinishSpawningActor(SpawnedTreeActor, FTransform(FRotator::ZeroRotator, LocationData.ObjectPosition));

		// TODO Add the tree actor to a map so I can update the collision and remove it later on
		WTSR->AddSpawnedTrees(LocationData.ObjectWorldCoords, SpawnedTreeActor);
	} else {
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Tree Actor!"));
	}
}

void AChunkWorld::SpawnGrass(FVoxelObjectLocationData LocationData) {
	UCustomProceduralMeshComponent* Mesh = NewObject<UCustomProceduralMeshComponent>(this);
	Mesh->RegisterComponent();
	Mesh->SetCastShadow(WTSR->GrassShadow);

	FVoxelObjectMeshData* MeshData = WTSR->GetRandomGrassMeshData();
	Mesh->CreateMeshSection(0, MeshData->Vertices, MeshData->Triangles, MeshData->Normals, MeshData->UV0, MeshData->Colors, TArray<FProcMeshTangent>(), true);

	// Set up simplified collision
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);

	//Mesh->ComponentTags.Add(TEXT("Grass"));

	// Setting custom data 
	Mesh->MeshType = MeshType::Grass;
	Mesh->ObjectWorldCoords = LocationData.ObjectWorldCoords;

	// Load and apply basic material to the mesh
	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/VoxelBasicMaterial.VoxelBasicMaterial"));

	if (Material) {
		Mesh->SetMaterial(0, Material);
	}

	Mesh->SetWorldLocation(LocationData.ObjectPosition);
	Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// Adding the grass object to a map so I can remove it later on
	WTSR->AddSpawnedGrass(LocationData.ObjectWorldCoords, Mesh);
}

void AChunkWorld::SpawnFlower(FVoxelObjectLocationData LocationData) {
	UCustomProceduralMeshComponent* Mesh = NewObject<UCustomProceduralMeshComponent>(this);
	Mesh->RegisterComponent();
	Mesh->SetCastShadow(WTSR->FlowerShadow);

	FVoxelObjectMeshData* MeshData = WTSR->GetRandomFlowerMeshData();
	Mesh->CreateMeshSection(0, MeshData->Vertices, MeshData->Triangles, MeshData->Normals, MeshData->UV0, MeshData->Colors, TArray<FProcMeshTangent>(), true);

	// Set up simplified collision
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);

	//Mesh->ComponentTags.Add(TEXT("Flower"));

	// Setting custom data 
	Mesh->MeshType = MeshType::Flower;
	Mesh->ObjectWorldCoords = LocationData.ObjectWorldCoords;

	// Load and apply basic material to the mesh
	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/VoxelBasicMaterial.VoxelBasicMaterial"));

	if (Material) {
		Mesh->SetMaterial(0, Material);
	}

	Mesh->SetWorldLocation(LocationData.ObjectPosition);
	Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// Adding the tree object to a map so I can remove it later on
	WTSR->AddSpawnedFlower(LocationData.ObjectWorldCoords, Mesh);
}

void AChunkWorld::SpawnNPC(TPair<FVoxelObjectLocationData, AnimalType> LocationAndType) {
	// Spawn the NPC actor deferred
	ABasicNPC* SpawnedNPCActor = GetWorld()->SpawnActorDeferred<ABasicNPC>(NPC, FTransform(FRotator::ZeroRotator, LocationAndType.Key.ObjectPosition), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (SpawnedNPCActor) {
		// Add references to the NPC and pass the pathfinding manager for making requests
		SpawnedNPCActor->SetWorldTerrainSettings(WTSR);
		SpawnedNPCActor->SetChunkLocationData(CLDR);
		SpawnedNPCActor->SetAnimationSettingsNPC(AnimS);
		SpawnedNPCActor->SetPathfindingManager(PathfindingManager);
		SpawnedNPCActor->SetNPCWorldLocation(LocationAndType.Key.ObjectWorldCoords);
		SpawnedNPCActor->InitializeBrain(LocationAndType.Value);
		SpawnedNPCActor->SetStatsVoxelsMeshNPC(SVMNpc);

		// Finish spawning the chunk actor
		UGameplayStatics::FinishSpawningActor(SpawnedNPCActor, FTransform(FRotator::ZeroRotator, LocationAndType.Key.ObjectPosition));

		// This occupies the current voxel where the NPC is spawned (used for collision avoidance during pathfinding)
		CLDR->AddOccupiedVoxelPosition(LocationAndType.Key.ObjectPosition, SpawnedNPCActor);

		// Add the NPC actor to a map so I can update the collision and remove it later on
		WTSR->AddSpawnedNpc(LocationAndType.Key.ObjectWorldCoords, SpawnedNPCActor);
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

	// Remove remaining grass to spawn position at current chunk destroyed
	CLDR->RemoveGrassSpawnPosition(destroyPosition);

	// Remove grass to spawn in the ChunkWorld cache
	GrassPositionsToSpawn.RemoveAll([&](const FVoxelObjectLocationData& Item) {
		return Item.ObjectWorldCoords == destroyPosition;
		});

	// Remove remaining flower to spawn position at current chunk destroyed
	CLDR->RemoveFlowerSpawnPosition(destroyPosition);

	// Remove flower to spawn in the ChunkWorld cache
	FlowerPositionsToSpawn.RemoveAll([&](const FVoxelObjectLocationData& Item) {
		return Item.ObjectWorldCoords == destroyPosition;
		});

	// Remove remaining NPC to spawn position at current chunk destroyed
	CLDR->RemoveNPCSpawnPosition(destroyPosition);

	// Remove NPCs to spawn in the ChunkWorld cache
	NPCPositionsToSpawn.RemoveAll([&](const TPair<FVoxelObjectLocationData, AnimalType>& Item) {
		return Item.Key.ObjectWorldCoords == destroyPosition;
		});
}

void AChunkWorld::DestroyTreeActors() {
	// Remove tree actors
	int removedTreeCounter = 0;
	while (!TreeActorsToRemove.IsEmpty() && removedTreeCounter < treesToRemovePerFrame) {

		ATree* treeToRemove = nullptr;
		TreeActorsToRemove.Peek(treeToRemove);

		if (!IsValid(treeToRemove)) {
			TreeActorsToRemove.Dequeue(treeToRemove);
			removedTreeCounter++;
			continue;
		}

		TreeActorsToRemove.Dequeue(treeToRemove);
		treeToRemove->Destroy();
		WTSR->TreeCount--;
		removedTreeCounter++;
	}
}

void AChunkWorld::DestroyGrassActors() {
	// Remove grass actors
	int removedGrassCounter = 0;
	while (!GrassActorsToRemove.IsEmpty() && removedGrassCounter < grassToRemovePerFrame) {

		UCustomProceduralMeshComponent* grassToRemove = nullptr;
		GrassActorsToRemove.Peek(grassToRemove);

		if (!IsValid(grassToRemove)) {
			GrassActorsToRemove.Dequeue(grassToRemove);
			removedGrassCounter++;
			continue;
		}

		GrassActorsToRemove.Dequeue(grassToRemove);
		grassToRemove->DestroyComponent();
		WTSR->GrassCount--;
		removedGrassCounter++;
	}
}

void AChunkWorld::DestroyFlowerActors() {
	// Remove flower actors
	int removedFlowerCounter = 0;
	while (!FlowerActorsToRemove.IsEmpty() && removedFlowerCounter < flowerToRemovePerFrame) {
		UCustomProceduralMeshComponent* flowerToRemove = nullptr;
		FlowerActorsToRemove.Peek(flowerToRemove);

		if (!IsValid(flowerToRemove)) {
			FlowerActorsToRemove.Dequeue(flowerToRemove);
			removedFlowerCounter++;
			continue;
		}

		FlowerActorsToRemove.Dequeue(flowerToRemove);
		flowerToRemove->DestroyComponent();
		WTSR->FlowerCount--;
		removedFlowerCounter++;
	}
}

void AChunkWorld::DestroyNpcActors() {
	// Remove flower actors
	int removedNpcCounter = 0;
	while (!NpcActorsToRemove.IsEmpty() && removedNpcCounter < npcToRemovePerFrame) {
		ABasicNPC* npcToRemove = nullptr;
		NpcActorsToRemove.Peek(npcToRemove);

		if (!IsValid(npcToRemove)) {
			NpcActorsToRemove.Dequeue(npcToRemove);
			removedNpcCounter++;
			continue;
		}

		NpcActorsToRemove.Dequeue(npcToRemove);
		npcToRemove->Destroy();
		WTSR->NPCCount--;
		removedNpcCounter++;
	}
}

void AChunkWorld::SpawnMultipleGrassObjects() {
	// Append grass positions waiting to be spawned
	TArray<FVoxelObjectLocationData> grassSpawnPositions = CLDR->getGrassSpawnPositionInRange();
	GrassPositionsToSpawn.Append(grassSpawnPositions);

	// Spawn a few trees in the current frame
	int spawnedGrassCounter = 0;
	for (int positionIndex = 0; positionIndex < GrassPositionsToSpawn.Num();) {
		if (spawnedGrassCounter >= grassToSpawnPerFrame) {
			return;
		}

		// Check if the grass position is still in range, otherwise discard it
		bool isGrassStillInRange = VegetationChunkSpawnPoints.Contains(GrassPositionsToSpawn[positionIndex].ObjectWorldCoords);
		if (isGrassStillInRange) {
			SpawnGrass(GrassPositionsToSpawn[positionIndex]);
			WTSR->GrassCount++;
		}

		// Print the grass count every 50
		/*if (WTSR->GrassCount % 1000 == 0) {
			UE_LOG(LogTemp, Log, TEXT("Grass count: %d"), WTSR->GrassCount);
		}*/

		GrassPositionsToSpawn.RemoveAt(positionIndex);
		spawnedGrassCounter++;
	}
}

void AChunkWorld::SpawnMultipleFlowerObjects() {
	// Append flower positions waiting to be spawned
	TArray<FVoxelObjectLocationData> flowerSpawnPositions = CLDR->getFlowerSpawnPositionInRange();
	FlowerPositionsToSpawn.Append(flowerSpawnPositions);

	// Spawn a few flowers in the current frame
	int spawnedFlowerCounter = 0;
	for (int positionIndex = 0; positionIndex < FlowerPositionsToSpawn.Num();) {
		if (spawnedFlowerCounter >= flowerToSpawnPerFrame) {
			return;
		}

		// Check if the flower position is still in range, otherwise discard it
		bool isFlowerStillInRange = VegetationChunkSpawnPoints.Contains(FlowerPositionsToSpawn[positionIndex].ObjectWorldCoords);
		if (isFlowerStillInRange) {
			SpawnFlower(FlowerPositionsToSpawn[positionIndex]);
			WTSR->FlowerCount++;
		}

		// Print the flower count every 50
		/*if (WTSR->FlowerCount % 50 == 0) {
			UE_LOG(LogTemp, Log, TEXT("Flower count: %d"), WTSR->FlowerCount);
		}*/

		FlowerPositionsToSpawn.RemoveAt(positionIndex);
		spawnedFlowerCounter++;
	}

}

void AChunkWorld::SpawnMultipleNpcObjects() {
	// Append NPC positions waiting to be spawned
	TArray<TPair<FVoxelObjectLocationData, AnimalType>> NPCSpawnPositions = CLDR->getNPCSpawnPositionInRange();
	NPCPositionsToSpawn.Append(NPCSpawnPositions);

	// Spawn a few flowers in the current frame
	int spawnedNPCCounter = 0;
	for (int positionIndex = 0; positionIndex < NPCPositionsToSpawn.Num();) {
		if (spawnedNPCCounter >= npcToSpawnPerFrame) {
			break;
		}

		bool isNpcStillInRange = NpcChunkSpawnPoints.Contains(NPCPositionsToSpawn[positionIndex].Key.ObjectWorldCoords);
		if (isNpcStillInRange) {
			SpawnNPC(NPCPositionsToSpawn[positionIndex]);
			WTSR->NPCCount++;
		}

		// Print the NPC count every 50
		if (WTSR->NPCCount % 10 == 0) {
			// UE_LOG(LogTemp, Log, TEXT("NPC count: %d"), WTSR->NPCCount); // TODO Uncomment after testing
		}

		NPCPositionsToSpawn.RemoveAt(positionIndex);
		spawnedNPCCounter++;
	}
}

void AChunkWorld::SpawnMultipleTreeObjects(const FVector& PlayerPosition) {
	// Append tree positions waiting to be spawned
	TArray<FVoxelObjectLocationData> treeSpawnPositions = CLDR->getTreeSpawnPositionsInRange();
	TreePositionsToSpawn.Append(treeSpawnPositions);

	int spawnedTreeCounter = 0;
	for (int32 positionIndex = 0; positionIndex < TreePositionsToSpawn.Num();) {
		if (spawnedTreeCounter >= treesToSpawnPerFrame) {
			spawnedTreesThisFrame = true;
			break;
		}

		// Check if the tree position is still in range, otherwise discard it
		bool isTreeStillInRange = TreeChunkSpawnPoints.Contains(TreePositionsToSpawn[positionIndex].ObjectWorldCoords);
		if (isTreeStillInRange) {
			SpawnTrees(TreePositionsToSpawn[positionIndex], PlayerPosition);
			WTSR->TreeCount++;
		}

		// Print the tree count every 50
		if (WTSR->TreeCount % 1000 == 0) {
			UE_LOG(LogTemp, Log, TEXT("Tree count: %d"), WTSR->TreeCount);
		}

		TreePositionsToSpawn.RemoveAt(positionIndex);
		spawnedTreeCounter++;
	}
}

void AChunkWorld::UpdateChunksCollision() {
	// Enabling and disabling collision for chunks 
	ABinaryChunk* removeCollisionChunk = WTSR->GetChunkToRemoveCollision();
	ABinaryChunk* enableCollisionChunk = WTSR->GetChunkToEnableCollision();

	if (IsValid(removeCollisionChunk) && removeCollisionChunk->IsActorInitialized()) {
		removeCollisionChunk->UpdateCollision(false);
	}

	if (IsValid(enableCollisionChunk) && enableCollisionChunk->IsActorInitialized()) {
		enableCollisionChunk->UpdateCollision(true);
	}
}

void AChunkWorld::UpdateTreesCollision() {
	// Enabling and disabling collision for trees 
	ATree* removeCollisionTree = WTSR->GetTreeToRemoveCollision();
	ATree* enableCollisionTree = WTSR->GetTreeToEnableCollision();

	if (IsValid(removeCollisionTree) && removeCollisionTree->IsActorInitialized()) {
		removeCollisionTree->UpdateCollision(false);
	}

	if (IsValid(enableCollisionTree) && enableCollisionTree->IsActorInitialized()) {
		enableCollisionTree->UpdateCollision(true);
	}
}

void AChunkWorld::SpawnSingleChunk(const FVector& PlayerPosition) {
	if (!CLDR->isMeshWaitingToBeSpawned()) {
		return;
	}

	spawnedChunksThisFrame = true;

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

void AChunkWorld::DestroySingleChunk() {
	FIntPoint chunkToDestroyPosition{};
	bool doesDestroyPositionExist = CLDR->getChunkToDestroyPosition(chunkToDestroyPosition);

	// I need to check if the destroy position exists in the map, otherwise I need to push it back 
	if (!doesDestroyPositionExist) {
		return;
	}

	AActor* chunkToRemove = WTSR->GetAndRemoveChunkFromMap(chunkToDestroyPosition);
	if (IsValid(chunkToRemove)) {
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

// Update the player's current position that will be used for pathfinding
void AChunkWorld::updatePlayerCurrentPosition(FVector& PlayerPosition) {
	if (updatePlayerCurrentPositionCounter >= updatePlayerCurrentPositionPerFrames) {
		WTSR->updateCurrentPlayerPosition(PlayerPosition);
		updatePlayerCurrentPositionCounter = 0;
	} else {
		updatePlayerCurrentPositionCounter++;
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

	if (PerlinNoiseSettingsRef) {
		PerlinNoiseSettingsRef = nullptr;
	}

	if (WorldTerrainSettingsRef) {
		WorldTerrainSettingsRef = nullptr;
	}

	if (ChunkLocationDataRef) {
		ChunkLocationDataRef = nullptr;
	}

	if (AnimationSettingsRef) {
		AnimationSettingsRef = nullptr;
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

	spawnedTreesThisFrame = false;
	spawnedChunksThisFrame = false;

	// Print the average chunk mesh compute time
	if (WTSR->chunksMeshCounter % 100 == 0 && WTSR->chunksMeshCounter != lastLoggedChunkCount) {
		float chunkSpawnTime = WTSR->chunkSpawnTime.count() / WTSR->chunksMeshCounter;
		int seconds = static_cast<int>(chunkSpawnTime) / 1000;
		int milliseconds = static_cast<int>(chunkSpawnTime) % 1000;

		UE_LOG(LogTemp, Warning, TEXT("Average mesh compute time for %d chunks: %d seconds, %d milliseconds."), WTSR->chunksMeshCounter, seconds, milliseconds);

		lastLoggedChunkCount = WTSR->chunksMeshCounter;
	}

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

	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

	// Update the player's current position so NPCs can use it for pathfinding
	updatePlayerCurrentPosition(PlayerPosition);

	const FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	const FIntPoint InitialChunkCoords = GetChunkCoordinates(WTSR->getInitialPlayerPosition());

	const bool isPlayerMovingOnAxisX = PlayerChunkCoords.X != InitialChunkCoords.X;
	const bool isPlayerMovingOnAxisZ = PlayerChunkCoords.Y != InitialChunkCoords.Y;

	if (!isLocationTaskRunning && (isPlayerMovingOnAxisX || isPlayerMovingOnAxisZ)) {
		isLocationTaskRunning.AtomicSet(true);
		chunksLocationRunnable = new ChunksLocationRunnable(PlayerPosition, WTSR, CLDR, &GrassActorsToRemove, &FlowerActorsToRemove, &TreeActorsToRemove, &NpcActorsToRemove);
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

	// Spawn chunk if there is a calculated mesh data waiting
	SpawnSingleChunk(PlayerPosition);

	UpdateChunksCollision();

	// Destroy a chunk and remove it from the map if there is a destroy position in queue
	DestroySingleChunk();

	// Reduce computing per frame by returning early if a chunk already got spawned this frame
	if (spawnedChunksThisFrame) {
		return;
	}

	// Check every few frames for spawned points in range and for vegetation not in range
	if (FramesCounterCheckSpawnedPointsInRange > FramesToCheckForSpawnPointsInRange) {
		CLDR->CheckForSpawnPointsInRange();
		CLDR->CheckAndAddVegetationNotInRange(&GrassActorsToRemove, &FlowerActorsToRemove);
		CLDR->CheckAndAddTreesNotInRange(&TreeActorsToRemove);
		CLDR->CheckAndAddNpcsNotInRange(&NpcActorsToRemove);
		FramesCounterCheckSpawnedPointsInRange = 0;

		// Get an updated vegetation and tree chunk spawn points
		VegetationChunkSpawnPoints = CLDR->GetVegetationChunkSpawnPoints();
		TreeChunkSpawnPoints = CLDR->GetTreeChunkSpawnPoints();
		NpcChunkSpawnPoints = CLDR->GetNpcChunkSpawnPoints();
	}
	FramesCounterCheckSpawnedPointsInRange++;


	// Spawn and remove a few Tree objects
	SpawnMultipleTreeObjects(PlayerPosition);
	DestroyTreeActors();

	// Update tree collision
	UpdateTreesCollision();

	// Uncomment to use the testing configurations instead
	// UseTestingConfigurations(ConfigToRun::NotificationAttackFoodSource);

	if (!UsingTestConfiguration) {
		SpawnMultipleGrassObjects();
		SpawnMultipleFlowerObjects();
		SpawnMultipleNpcObjects();
	}

	// Reduce computing per frame by returning early if a tree already got spawned this frame
	if (spawnedTreesThisFrame) {
		return;
	}

	// Destroy a few vegetation actors and NPCs
	DestroyGrassActors();
	DestroyFlowerActors();
	DestroyNpcActors();
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
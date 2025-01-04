// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunksLocationRunnable.h"
#include "..\SingleChunk\BinaryChunk.h"
#include "..\Vegetation\Trees\Tree.h"
#include "..\TerrainSettings\WorldTerrainSettings.h"
#include "..\ChunkData\ChunkLocationData.h"

ChunksLocationRunnable::ChunksLocationRunnable(FVector PlayerPosition, UWorldTerrainSettings* InWorldTerrainSettingsRef, UChunkLocationData* InChunkLocationDataRef) : PlayerPosition(PlayerPosition), isRunning(false), isTaskComplete(false) {
	WorldTerrainSettingsRef = InWorldTerrainSettingsRef;
	ChunkLocationDataRef = InChunkLocationDataRef;
}

ChunksLocationRunnable::~ChunksLocationRunnable() {
}

bool ChunksLocationRunnable::Init() {
	isRunning = true;
	isTaskComplete = false;
	return true;
}

uint32 ChunksLocationRunnable::Run() {
	while (isRunning) {
		// Spawn new chunks
		WTSR->UpdateChunkSemaphore->Acquire();
		UpdateChunks();
		WTSR->UpdateChunksCollision(PlayerPosition);
		WTSR->UpdateTreeCollisions(PlayerPosition);
		//UpdateChunkCollisions();
		WTSR->UpdateChunkSemaphore->Release();

		isTaskComplete = true;
		isRunning = false;
	}
	return 0;
}

void ChunksLocationRunnable::Stop() {
	isRunning = false;
}

void ChunksLocationRunnable::Exit() {
}

FThreadSafeBool ChunksLocationRunnable::IsTaskComplete() const {
	return isTaskComplete;
}

void ChunksLocationRunnable::UpdateChunks() {
	FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	FIntPoint InitialChunkCoords = GetChunkCoordinates(WTSR->getInitialPlayerPosition());

	// Add and remove chunks on the X axis 
	if (PlayerChunkCoords.X != InitialChunkCoords.X) {

		int lastRowX{ 0 };
		int newRowX{ 0 };
		int lastTreeRowX{ 0 };
		int newTreeRowX{ 0 };
		if (PlayerChunkCoords.X > InitialChunkCoords.X) {
			// Get the first row by adding the draw distance to the player's X
			lastRowX = PlayerChunkCoords.X - WTSR->DrawDistance;
			newRowX = PlayerChunkCoords.X + WTSR->DrawDistance;
			lastTreeRowX = PlayerChunkCoords.X - WTSR->TreeSpawnRadius;
			newTreeRowX = PlayerChunkCoords.X + WTSR->TreeSpawnRadius;
		} else {
			// Get the last row by substracting the draw distance from the the initial X
			lastRowX = InitialChunkCoords.X + WTSR->DrawDistance;
			newRowX = InitialChunkCoords.X - WTSR->DrawDistance;
			lastTreeRowX = PlayerChunkCoords.X + WTSR->TreeSpawnRadius;
			newTreeRowX = PlayerChunkCoords.X - WTSR->TreeSpawnRadius;
		}

		// Add new chunks and remove old chunks based on the player's new X position
		const int firstIndexChunkZ = InitialChunkCoords.Y - WTSR->DrawDistance;
		const int lastIndexChunkZ = InitialChunkCoords.Y + WTSR->DrawDistance;

		// Loop and remove the entire row of chunks 
		for (int z = firstIndexChunkZ; z < lastIndexChunkZ; z++) {
			FIntPoint oldChunkCoords = FIntPoint(lastRowX, z);

			CLDR->addChunksToDestroyPosition(oldChunkCoords);
			
			// Adding new row 
			FIntPoint newChunkCoords = FIntPoint(newRowX, z);
			FVector ChunkPosition = FVector(newRowX * WTSR->chunkSize * WTSR->UnrealScale, z * WTSR->chunkSize * WTSR->UnrealScale, 0);

			CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, newChunkCoords));

			// Add world coordinates for tree spawning if is inside the tree radius
			// TODO I MIGHT HAVE TO JUST CHECK FOR THE CURRENT AXIS, NOT BOTH
			// TODO CHECK IF I SHOULD PASS PlayerChunkCoords, as this might be incorrect
			if (WTSR->isPointWithinTreeRadiusRange(PlayerChunkCoords)) { 
				// At the old chunk coordinates, add any tree actors to a destroy list
				FIntPoint oldTreeCoords = FIntPoint(lastTreeRowX, z);
				FIntPoint newTreeCoords = FIntPoint(newTreeRowX, z);

				CLDR->addTreeToDestroyPosition(oldTreeCoords);
			
				// Update tree spawn radius
				WTSR->RemovePlayer2DTreeRadiusPoint(oldTreeCoords);
				WTSR->AddPlayer2DTreeRadiusPoint(newTreeCoords);
			}
		}
	}

	// Update the initial position for the next frame
	PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	InitialChunkCoords = GetChunkCoordinates(WTSR->getInitialPlayerPosition());

	// Add and remove chunks on the Y axis 
	if (PlayerChunkCoords.Y != InitialChunkCoords.Y) {
		// Add new chunks and remove old chunks based on the player's new Y position
		int lastRowZ{ 0 };
		int newRowZ{ 0 };
		int lastTreeRowZ{ 0 };
		int newTreeRowZ{ 0 };
		if (PlayerChunkCoords.Y > InitialChunkCoords.Y) {
			// Get the first row by adding the draw distance to the player's Z
			lastRowZ = InitialChunkCoords.Y - WTSR->DrawDistance;
			newRowZ = InitialChunkCoords.Y + WTSR->DrawDistance;
			lastTreeRowZ = PlayerChunkCoords.Y - WTSR->TreeSpawnRadius;
			newTreeRowZ = PlayerChunkCoords.Y + WTSR->TreeSpawnRadius;
		} else {
			// Get the last row by substracting the draw distance from the the initial Z
			lastRowZ = PlayerChunkCoords.Y + WTSR->DrawDistance;
			newRowZ = PlayerChunkCoords.Y - WTSR->DrawDistance;
			lastTreeRowZ = PlayerChunkCoords.Y + WTSR->TreeSpawnRadius;
			newTreeRowZ = PlayerChunkCoords.Y - WTSR->TreeSpawnRadius;
		}

		const int firstIndexChunkX = PlayerChunkCoords.X - WTSR->DrawDistance + 1;
		const int lastIndexChunkX = PlayerChunkCoords.X + WTSR->DrawDistance + 1;

		// Loop and remove the entire row of chunks 
		for (int x = firstIndexChunkX; x < lastIndexChunkX; x++) {
			FIntPoint oldChunkCoords = FIntPoint(x, lastRowZ);
			CLDR->addChunksToDestroyPosition(oldChunkCoords);

			FIntPoint newChunkCoords = FIntPoint(x, newRowZ);
			FVector ChunkPosition = FVector(x * WTSR->chunkSize * WTSR->UnrealScale, newRowZ * WTSR->chunkSize * WTSR->UnrealScale, 0);

			CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, newChunkCoords));

			// Add world coordinates for tree spawning if is inside the tree radius
			// TODO I MIGHT HAVE TO JUST CHECK FOR THE CURRENT AXIS, NOT BOTH
			// TODO CHECK IF I SHOULD PASS PlayerChunkCoords, as this might be incorrect
			if (WTSR->isPointWithinTreeRadiusRange(PlayerChunkCoords)) {
				FIntPoint oldTreeCoords = FIntPoint(x, lastTreeRowZ);
				FIntPoint newTreeCoords = FIntPoint(x, newTreeRowZ);

				// At the old chunk coordinates, add any tree actors to a destroy list
				CLDR->addTreeToDestroyPosition(oldTreeCoords);

				// Update tree spawn radius
				WTSR->RemovePlayer2DTreeRadiusPoint(oldTreeCoords);
				WTSR->AddPlayer2DTreeRadiusPoint(newTreeCoords);
			}
		}

	}

	// Update the initial position for the next frame
	WTSR->updateInitialPlayerPosition(PlayerPosition);
}

/*
* Get read-only items of SpawnedChunksMap and iterate to see if chunks are outside or inside of
* the collision threshold. Chunks inside the threshold will have their meshes regenerated with
* collision and chunks outside of it will get their collision disabled through a mesh update.
*/
void ChunksLocationRunnable::UpdateChunkCollisions() {
	for (const TPair<FIntPoint, AActor*>& ChunkPair : WTSR->GetSpawnedChunksMap()) {
		ABinaryChunk* ChunkActor = Cast<ABinaryChunk>(ChunkPair.Value);

		if (ChunkActor) {
			const FVector ChunkPosition = ChunkActor->GetActorLocation();

			// Define the boundaries for the collision check
			float minX = PlayerPosition.X - WTSR->CollisionDistance;
			float maxX = PlayerPosition.X + WTSR->CollisionDistance;
			float minY = PlayerPosition.Y - WTSR->CollisionDistance;
			float maxY = PlayerPosition.Y + WTSR->CollisionDistance;

			// Check if the player is within the collision boundaries
			bool withinCollisionDistance = (ChunkPosition.X >= minX && ChunkPosition.X <= maxX) &&
				(ChunkPosition.Y >= minY && ChunkPosition.Y <= maxY);

			// Update collision state based on proximity
			if (withinCollisionDistance) {
				if (!ChunkActor->HasCollision()) {
					WTSR->AddChunkToEnableCollision(ChunkActor);
				}
			} else {
				if (ChunkActor->HasCollision()) {
					WTSR->AddChunkToRemoveCollision(ChunkActor);
				}
			}
		}
	}
}

void ChunksLocationRunnable::UpdateTreeCollisions() {
	for (const TPair<FIntPoint, TArray<ATree*>>& treesAtLocation : WTSR->GetSpawnedTreesMap()) {
		// Iterate through each tree in the array for this specific location
		for (ATree* tree : treesAtLocation.Value) {
			if (tree) {
				const FVector ChunkPosition = tree->GetActorLocation();

				// Define the boundaries for the collision check
				float minX = PlayerPosition.X - WTSR->VegetationCollisionDistance;
				float maxX = PlayerPosition.X + WTSR->VegetationCollisionDistance;
				float minY = PlayerPosition.Y - WTSR->VegetationCollisionDistance;
				float maxY = PlayerPosition.Y + WTSR->VegetationCollisionDistance;

				// Check if the player is within the collision boundaries
				bool withinCollisionDistance = (ChunkPosition.X >= minX && ChunkPosition.X <= maxX) &&
					(ChunkPosition.Y >= minY && ChunkPosition.Y <= maxY);

				// Update collision state based on proximity
				if (withinCollisionDistance) {
					if (!tree->HasCollision()) {
						WTSR->AddTreeToEnableCollision(tree);
					}
				} else {
					if (tree->HasCollision()) {
						WTSR->AddTreeToRemoveCollision(tree);
					}
				}
			}
		}
	}
}

FIntPoint ChunksLocationRunnable::GetChunkCoordinates(FVector Position) const {
	const int32 ChunkX = FMath::FloorToInt(Position.X / (WTSR->chunkSize * WTSR->UnrealScale));
	const int32 ChunkY = FMath::FloorToInt(Position.Y / (WTSR->chunkSize * WTSR->UnrealScale));
	return FIntPoint(ChunkX, ChunkY);
}

void ChunksLocationRunnable::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void ChunksLocationRunnable::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}




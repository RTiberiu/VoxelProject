# pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include <GameFramework/FloatingPawnMovement.h>
#include <Runtime/AIModule/Classes/AIController.h>

#include "..\..\Pathfinding\SearchLibrary\Path.h"
#include "..\..\Pathfinding\PathfindingThreadPool\PathfindingThreadManager.h"

#include "BasicNPC.generated.h"

class PathfindingThreadManager;
class UWorldTerrainSettings;

UCLASS()
class ABasicNPC : public APawn {
	GENERATED_BODY()

public:
	ABasicNPC();
	~ABasicNPC();

	void SetNPCWorldLocation(FIntPoint InNPCWorldLocation);

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetPathfindingManager(PathfindingThreadManager* InPathfindingManager);
	void SetChunkLocationData(UChunkLocationData* InChunkLocationData);

	void SetPathToPlayerAndNotify(Path* InPathToPlayer);

private:
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	UChunkLocationData* ChunkLocationDataRef;
	UChunkLocationData*& CLDR = ChunkLocationDataRef;

	PathfindingThreadManager* PathfindingManager;

	AAIController* AIController;

	void spawnNPC();
	void buildAnimationsList();

	void PlayAnimation(const FString& type);

	void RequestPathToPlayer();
	Path* pathToPlayer;
	bool pathIsReady;

	void ConsumePathAndMoveToLocation();

	void SetTargetLocation();
	bool IsTargetLocationAvailable();

	void TimelineProgress(float Value);

	FIntPoint NPCWorldLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* SkeletalMesh;

	UFloatingPawnMovement* FloatingMovement;

	static constexpr int animationChangeAfterFrames = 180;
	int animationFrameCounter = 0;

	// TArray<FString> Animations;
	TMap<FString, UAnimSequence*> Animations;

	FVector currentLocation;
	FVector* targetLocation;
	FVector timelineStartPos;
	FVector timeLineEndPos;

	bool targetLocationIsAvailable;

	float movementSpeed;

	void AdjustRotationTowardsNextLocation(const FVector& actorLocation, const FVector& targetPosition, const float& deltaTime);

	bool isJumping;
	float jumpProgress;
	FVector jumpStart;
	FVector jumpEnd;
	const float jumpHeight = 60.0f;
	const float jumpSpeed = 2.0f;
	FString currentAnimPlaying;

	bool waitForNextPositionCheck;
	float OccupiedDelayTimer = 0.0f; // Accumulates time when target location is occupied by another NPC
	const float OccupiedDelayThreshold = 0.5f; // Delay in seconds before trying again to move to the next location

	// TESTING TICK CALLS
	float DelayBeforeFirstPathRequest;
	float TimeSinceLastCall;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;



};
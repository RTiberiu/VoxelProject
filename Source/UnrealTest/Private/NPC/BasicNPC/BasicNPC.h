# pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include <Components/SphereComponent.h>
#include <GameFramework/FloatingPawnMovement.h>
#include <Runtime/AIModule/Classes/AIController.h>

#include "..\SettingsNPC\AnimationSettingsNPC.h"
#include "..\SettingsNPC\ActionStructures.h"
//#include "DecisionSystemNPC.h"

#include "..\StatsNPC\StatsVoxelsMeshNPC.h"

#include "..\..\Utils\CustomMesh\CustomProceduralMeshComponent.h"
#include <variant>
#include "..\..\Pathfinding\SearchLibrary\Path.h"
#include "..\..\Pathfinding\PathfindingThreadPool\PathfindingThreadManager.h"
#include "BasicNPC.generated.h"

class UDecisionSystemNPC;
class PathfindingThreadManager;
class UWorldTerrainSettings;

enum VisionList {
	Threat,
	Allies,
	NpcFood,
	FoodSource
};

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
	void SetAnimationSettingsNPC(UAnimationSettingsNPC* InAnimationSettingsNPCRef); 
	void SetStatsVoxelsMeshNPC(UStatsVoxelsMeshNPC* InStatsVoxelsMeshNPC);

	void InitializeBrain(const AnimalType& animalType);

	void SetPathToTargetAndNotify(Path* InPathToTarget);

	const AnimalType& GetType();
	const AnimalType& GetNpcFoodRelationships();
	const AnimalType& GetAlliesRelationships();
	const AnimalType& GetEnemiesRelationships();

	bool IsThreatInRange();
	bool IsAllyInRange();
	bool IsFoodNpcInRange();
	bool IsFoodSourceInRange();

	const FIntPoint& GetNpcWorldLocation();

	std::variant<ABasicNPC*, UCustomProceduralMeshComponent*> GetClosestInVisionList(VisionList list);

	FVector& GetCurrentLocation();

private:
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	UChunkLocationData* ChunkLocationDataRef;
	UChunkLocationData*& CLDR = ChunkLocationDataRef;

	UAnimationSettingsNPC* AnimationSettingsNPCRef;
	UAnimationSettingsNPC*& AnimS = AnimationSettingsNPCRef;

	UStatsVoxelsMeshNPC* StatsVoxelsMeshNPCRef;
	UStatsVoxelsMeshNPC*& SVMNpc = StatsVoxelsMeshNPCRef;

	PathfindingThreadManager* PathfindingManager;

	AAIController* AIController;

	UPROPERTY()
	UDecisionSystemNPC* DecisionSys;

	AnimalType NpcType;
	AnimalRelationships Relationships;

	void spawnNPC();

	// NPC Stats related methods
	void UpdateStatsVoxelsMesh(StatsType statType);
	int GetStatsVoxelNumber(const float& CurrentValue, const float& MaxValue);
	void InitializeStatsVoxelMeshes();
	TMap<StatsType, UCustomProceduralMeshComponent*> StatsMeshes;

	void PlayAnimation(const AnimationType& animationtype);

	void InitializeVisionCollisionSphere(const float& radius);

	void RequestPathToPlayer();
	Path* pathToTarget;
	bool pathIsReady;

	void ConsumePathAndMoveToLocation(const float& DeltaSeconds);

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

	void RunTargetAnimationAndUpdateAttributes(float& DeltaSeconds);

	bool runTargetAnimation;
	bool isTargetSet;
	FVector targetLocation;
	AnimationType animationToRunAtTarget;
	ActionType actionType;
	UObject* actionTarget;

	FVector timelineStartPos;
	FVector timeLineEndPos;

	// Counters for actions
	float EatingCounter;
	float RestCounter;
	float HungerCounter;

	bool targetLocationIsAvailable;

	void AdjustRotationTowardsNextLocation(const FVector& actorLocation, const FVector& targetPosition, const float& deltaTime);

	bool isJumping;
	float jumpProgress;
	FVector jumpStart;
	FVector jumpEnd;
	const float jumpHeight = 120.0f;
	const float jumpSpeed = 1.5f;
	const float delayJump = 0.12f;
	float delayJumpCounter = 0.0f;
	const float rotatingSpeed = 10.0f;
	AnimationType currentAnimPlaying;

	bool isWalking;
	float walkProgress;
	FVector walkStart;
	FVector walkEnd;

	void RemoveFoodTargetFromMapAndDestroy();
	
	// Methods to update the NPC attributes
	void UpdateFoodAttributes(const uint8& hungerRecovered, bool ateBasicFood);
	bool ForceRestWhenStaminaIsZero(const float& DeltaSeconds);
	void UpdateHunger(const float& DeltaSeconds);
	bool UpdateStamina(const float& DeltaSeconds, const uint8_t& Threshold);

	bool waitForNextPositionCheck;
	float OccupiedDelayTimer = 0.0f; // Accumulates time when target location is occupied by another NPC
	const float OccupiedDelayThreshold = 0.5f; // Delay in seconds before trying again to move to the next location

	// TESTING TICK CALLS
	float DelayBeforeFirstPathRequest;

	// Collision sphere settings for updating objects "visible" to the NPC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionNpcDetectionSphere;

	// Overlap event functions
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void AddOverlappingNpcToVisionList(ABasicNPC* OverlappingNpc);
	void RemoveOverlappingNpcFromVisionList(ABasicNPC* OverlappingNpc);

	void AddOverlappingBasicFoodSource(UPrimitiveComponent* OverlappingFood);
	void RemoveOverlappingBasicFoodSource(UPrimitiveComponent* OverlappingFood);

	// Helper functions to get the closest item in the vision lists
	ABasicNPC* GetClosestInList(const TArray<ABasicNPC*>& list);
	UCustomProceduralMeshComponent* GetClosestInList(const TArray<UCustomProceduralMeshComponent*>& list);

	// Avoid repeating the same compare the closest object logic
	template<typename T>
	T* GetClosestInListGeneric(const TArray<T*>& list, TFunctionRef<FVector(T*)> GetLocation) const {
		if (list.Num() == 0) {
			return nullptr;
		}

		T* closest = nullptr;
		float closestDistance = FLT_MAX;

		for (T* element : list) {
			if (element) {
				float distance = FVector::Dist(currentLocation, GetLocation(element));
				if (distance < closestDistance) {
					closestDistance = distance;
					closest = element;
				}
			}
		}
		return closest;
	}

	// Store objects in the NPC's perceptation sphere 
	TArray<ABasicNPC*> ThreatsInRange;
	TArray<ABasicNPC*> AlliesInRange;
	TArray<ABasicNPC*> FoodNpcInRange;
	TArray<UCustomProceduralMeshComponent*> FoodSourceInRange;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;



};
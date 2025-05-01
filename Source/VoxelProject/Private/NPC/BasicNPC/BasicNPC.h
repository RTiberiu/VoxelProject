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

enum class NotificationType : uint8{
	Default		= 1,
	Notifying	= 2,
	Accepted	= 3,
	Discarded	= 4
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

	std::variant<ABasicNPC*, UCustomProceduralMeshComponent*> GetClosestInVisionList(
		VisionList list, 
		bool ChooseOptimalAction, 
		const int& IncrementTargetInVisionList
	);

	FVector& GetCurrentLocation();

	// Methods for other NPCs to call to communicate
	void AttackAndReduceHealth(const int& damage, uint8_t attackerEatingSpeed, ABasicNPC* attacker);
	bool IsDead();
	void TriggerFoodRewardOnKill();

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
	void UpdateStatsVoxelsMesh(StatsType statType, NotificationType notificationType = NotificationType::Default);
	int GetStatsVoxelNumber(const float& CurrentValue, const float& MaxValue);
	void InitializeStatsVoxelMeshes();
	TMap<StatsType, UCustomProceduralMeshComponent*> StatsMeshes;

	void PlayAnimation(const AnimationType& animationtype, bool loopAnim = true);

	void InitializeVisionCollisionSphere(const float& radius);

	void RequestPathToPlayer();
	Path* pathToTarget;
	bool pathIsReady;

	void ConsumePathAndMoveToLocation(const float& DeltaSeconds);

	void SetTargetLocation();
	bool IsTargetLocationAvailable();

	void TimelineProgress(float Value);

	bool IsTargetLocationCloseEnough(FVector& current, FVector& target);

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
	void SignalEndOfAction();

	bool runTargetAnimation;
	bool isTargetSet;
	FVector targetLocation;
	AnimationType animationToRunAtTarget;
	ActionType actionType = ActionType::NoAction;
	UObject* actionTarget;

	FVector timelineStartPos;
	FVector timeLineEndPos;

	// Counters for actions
	float EatingCounter;
	float RestCounter;
	float HungerCounter;
	float AttackDelayCounter;
	bool delayNextAttack;

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

	// Control variables for making the NPC look around after reaching a target voxel
	bool isLookingAround = false;
	float lookAroundChance = 0.1f;
	float lookingAroundCounter = 0.0f;
	float lookingAroundThreshold = 1.0f;
	AnimationType lookingDirection;

	void RemoveFoodTargetFromMapAndDestroy();
	
	// Methods to update the NPC attributes
	void UpdateFoodAttributes(const uint8& hungerRecovered, bool ateBasicFood);
	bool ForceRestWhenStaminaIsZero(const float& DeltaSeconds);
	void UpdateHunger(const float& DeltaSeconds);
	bool UpdateStamina(const float& DeltaSeconds, const uint8_t& Threshold);

	// Trigger a death animation and destroy the NPC
    void TriggerNpcDeath(uint8_t attackerEatingSpeed = 10);
	void WaitForDespawnThresholdAndDestroy(const float& DeltaSeconds);
	bool isDeathTriggered;
	float DespawningCounter = 0.0f;
	uint8_t DespawnTime = 10.0f;

	bool waitForNextPositionCheck;
	bool checkNextPosition;
	float OccupiedDelayTimer = 0.0f; // Accumulates time when target location is occupied by another NPC
	const float OccupiedDelayThreshold = 0.5f; // Delay in seconds before trying again to move to the next location
	float FrustrationCounter = 0.0f; // Accumulates time when the NPC is not able to reach the target location
	const float FrustrationThreshold = 1.0f; // Time in seconds before the NPC gives up on moving to the target location
	bool frutrationTriggered = false; // Flag to check if the frustration threshold was reached
	int sameActionFrustrationCounter = 0; // Depending on this, the N-th target in the vision list will be selected
	UObject* LastActionTarget = nullptr; // Used to compare if the new action target is the same as the previous one

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
	ABasicNPC* GetClosestInList(const TArray<ABasicNPC*>& list, bool ChooseOptimalAction, const int& IncrementTargetInVisionList);
	UCustomProceduralMeshComponent* GetClosestInList(const TArray<UCustomProceduralMeshComponent*>& list, bool ChooseOptimalAction, const int& IncrementTargetInVisionList);

    // Avoid repeating the same compare the closest object logic  
	template<typename T>
	T* GetClosestInListGeneric(
		const TArray<T*>& list,
		TFunctionRef<FVector(T*)> GetLocation,
		bool ChooseOptimalAction,
		const int32& IncrementTargetInVisionList) const {
		if (list.Num() == 0) {
			return nullptr;
		}

		// Build an array of (distance, element) pairs
		TArray<TPair<float, T*>> distanceArray;
		distanceArray.Reserve(list.Num());
		for (T* element : list) {
			if (element) {
				float dist = FVector::Dist(currentLocation, GetLocation(element));
				distanceArray.Emplace(dist, element);
			}
		}

		// Sort by distance ascending
		distanceArray.Sort([](auto& A, auto& B) {
			return A.Key < B.Key;
			});

		if (ChooseOptimalAction) {
			// Return nullptr if the requested N-th target doesn't exist
			if (IncrementTargetInVisionList >= distanceArray.Num()) {
				return nullptr;
			}

			// Otherwise pick exactly that one
			const int32 TargetIndex = IncrementTargetInVisionList;
			T* chosen = distanceArray[TargetIndex].Value;
			float chosenDist = distanceArray[TargetIndex].Key;
			return chosen;
		} else {
			// return the second closest
			if (distanceArray.Num() < 2) {
				return nullptr;
			}
			return distanceArray[1].Value;
		}
	}

	// Store objects in the NPC's perceptation sphere 
	TArray<ABasicNPC*> ThreatsInRange;
	TArray<ABasicNPC*> AlliesInRange;
	TArray<ABasicNPC*> FoodNpcInRange;
	TArray<UCustomProceduralMeshComponent*> FoodSourceInRange;

	// Notify NPCs in the vision list based on the current action
	void NotifyNpcsAroundOfEvent(const NpcAction& CurrentAction);

	// Handles the notification, based on the action that triggered the notification
	void ReceiveNotificationOfEvent(const NpcAction& ActionTriggered);
	bool InterruptAction = false; // Used to trigger an interrupt when notified
	bool ShowNotificationStat = false; // Used to show the notification stat
	float ShowNotificationStatCounter = 0.0f;
	float ShowNotificationStatThreshold = 1.5f; // For how long to show the notification

	void ReplaceCurrentActionWithNotifiedAction(const NpcAction& ActionTriggered);
	bool AcceptAttackFoodSourceNotification();

	void TriggerPathfindingTask();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;



};
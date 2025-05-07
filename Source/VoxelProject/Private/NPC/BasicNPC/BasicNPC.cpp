#include "BasicNPC.h"

#include "..\..\Chunks\TerrainSettings\WorldTerrainSettings.h"
#include "DecisionSystemNPC.h"
#include "KismetProceduralMeshLibrary.h"
#include "..\..\Chunks\ChunkData\ChunkLocationData.h"

#include "GameFramework/PawnMovementComponent.h"

ABasicNPC::ABasicNPC() {
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	RootComponent = SkeletalMesh;

	pathToTarget = nullptr;
	pathIsReady = false;
	waitForNextPositionCheck = false;
	checkNextPosition = true;
	targetLocationIsAvailable = false;
	runTargetAnimation = false;
	isTargetSet = false;
	isDeathTriggered = false;

	ThreatsInRange = TArray<ABasicNPC*>();
	AlliesInRange = TArray<ABasicNPC*>();
	FoodNpcInRange = TArray<ABasicNPC*>();
	FoodSourceInRange = TArray<UCustomProceduralMeshComponent*>();

	UPawnMovementComponent* MovementComponent = FindComponentByClass<UPawnMovementComponent>();
	if (!MovementComponent) {
		FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
	}
}

ABasicNPC::~ABasicNPC() {
	// TODO Cleanup here
}

void ABasicNPC::SetNPCWorldLocation(FIntPoint InNPCWorldLocation) {
	NPCWorldLocation = InNPCWorldLocation;
}

void ABasicNPC::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void ABasicNPC::SetPathfindingManager(PathfindingThreadManager* InPathfindingManager) {
	PathfindingManager = InPathfindingManager;
}

void ABasicNPC::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}

void ABasicNPC::SetAnimationSettingsNPC(UAnimationSettingsNPC* InAnimationSettingsNPCRef) {
	AnimationSettingsNPCRef = InAnimationSettingsNPCRef;
}

// Should be called after InitializeBrain() and it sets the voxels above the NPC
// showing various attributes. 
void ABasicNPC::SetStatsVoxelsMeshNPC(UStatsVoxelsMeshNPC* InStatsVoxelsMeshNPC) {
	StatsVoxelsMeshNPCRef = InStatsVoxelsMeshNPC;

	// Creating all the meshes that will hold the stats mesh data based on parameters
	InitializeStatsVoxelMeshes();
}

void ABasicNPC::InitializeBrain(const AnimalType& animalType) {
	NpcType = animalType;
	Relationships = AnimalsRelationships[NpcType];

	// Create the decision system and initialize it
	DecisionSys = NewObject<UDecisionSystemNPC>();
	DecisionSys->Initialize(this, animalType);

	// Assign the sphere radius from the 
	InitializeVisionCollisionSphere(DecisionSys->AnimalAttributes.awarenessRadius);
}

void ABasicNPC::UpdateStatsVoxelsMesh(StatsType statType, NotificationType notificationType) {
	// Get the current and max value for the stat
	float CurrentValue{ 0 };
	float MaxValue{ 0 };
	int FillnessValue{ 0 };

	switch (statType) {
	case StatsType::Stamina:
		CurrentValue = static_cast<float>(DecisionSys->AnimalAttributes.currentStamina);
		MaxValue = static_cast<float>(DecisionSys->AnimalAttributes.maxStamina);
		break;
	case StatsType::Hunger:
		CurrentValue = static_cast<float>(DecisionSys->AnimalAttributes.currentHunger);
		MaxValue = static_cast<float>(DecisionSys->AnimalAttributes.maxHunger);
		break;
	case StatsType::HealthPoints:
		CurrentValue = static_cast<float>(DecisionSys->AnimalAttributes.currentHp);
		MaxValue = static_cast<float>(DecisionSys->AnimalAttributes.maxHp);
		break;
	case StatsType::FoodPouch:
		CurrentValue = static_cast<float>(DecisionSys->AnimalAttributes.foodPouch);
		MaxValue = static_cast<float>(DecisionSys->AnimalAttributes.maxHunger);
		break;
	case StatsType::AlliesInPack:
		break;
	case StatsType::Notification:
		FillnessValue = static_cast<uint8_t>(notificationType);
		break;
	};

	// Only get the fillness values for the other stat types
	if (statType != StatsType::Notification) {
		FillnessValue = GetStatsVoxelNumber(CurrentValue, MaxValue);
	}

	// Get the correct stat mesh and update it 
	UCustomProceduralMeshComponent* Mesh = StatsMeshes[statType];
	Mesh->ClearAllMeshSections();

	// Return early if filnnes is zero, since it should select any mesh
	if (FillnessValue == 0) {
		return;
	}

	FVoxelObjectMeshData* NewStatVoxelMeshData = SVMNpc->GetStatsMeshData(statType, FillnessValue);

	// This ensure the normals remain correct when the root (npc) rotates // TODO Potentially do this only once
	TArray<FVector> RecalculatedNormals;
	TArray<FProcMeshTangent> RecalculatedTangents;
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
		NewStatVoxelMeshData->Vertices,
		NewStatVoxelMeshData->Triangles,
		NewStatVoxelMeshData->UV0,
		RecalculatedNormals,
		RecalculatedTangents
	);

	// Use the recalculated normals and tangents and create the mesh section
	Mesh->CreateMeshSection(
		0,
		NewStatVoxelMeshData->Vertices,
		NewStatVoxelMeshData->Triangles,
		RecalculatedNormals,
		NewStatVoxelMeshData->UV0,
		NewStatVoxelMeshData->Colors,
		RecalculatedTangents,
		false
	);
}

// Calculate the fillness value for the stats voxel
int ABasicNPC::GetStatsVoxelNumber(const float& CurrentValue, const float& MaxValue) {
	if (CurrentValue <= 0) {
		return 0;
	}

	float Ratio = CurrentValue / MaxValue;
	int VoxelCount = FMath::RoundToInt(Ratio * 27.0f);
	return FMath::Clamp(VoxelCount, 0, 27);
}

void ABasicNPC::InitializeStatsVoxelMeshes() {
	FRotator VoxelRotation(-180.0f, 0.0f, 45.0f);

	TArray<StatsType> StatsTypes = SVMNpc->GetStatsTypes();
	for (StatsType statsType : StatsTypes) {
		UCustomProceduralMeshComponent* Mesh = NewObject<UCustomProceduralMeshComponent>(this);
		Mesh->RegisterComponent();
		Mesh->SetCastShadow(WTSR->NpcShadow);

		UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/VoxelBasicMaterial.VoxelBasicMaterial"));
		if (Material) {
			Mesh->SetMaterial(0, Material);
		}

		// Set the offset location and rotation
		Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

		FVector NewLocation = currentLocation + SVMNpc->GetStatsVoxelOffset(statsType);
		Mesh->SetRelativeRotation(VoxelRotation);
		Mesh->SetRelativeLocation(NewLocation);
		StatsMeshes.Add(statsType, Mesh);

		// Add initial values for the stats voxels
		UpdateStatsVoxelsMesh(statsType);
	}
}

void ABasicNPC::spawnNPC() {
	FString MeshPath = AnimS->GetSkeletalMeshPath(NpcType);

	// Load the skeletal mesh
	USkeletalMesh* LoadedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
	if (LoadedMesh) {
		SkeletalMesh->SetCastShadow(WTSR->NpcShadow);
		SkeletalMesh->SetSkeletalMesh(LoadedMesh);
		SkeletalMesh->SetWorldScale3D(FVector(0.65f, 0.65f, 0.65f)); // TODO I might want to scale this in the editor only once
	} else {
		UE_LOG(LogTemp, Error, TEXT("Couldn't find sekeletal mesh."));
	}

	// Load the animation asset
	PlayAnimation(AnimationType::IdleA);

	currentLocation = GetActorLocation();
}

void ABasicNPC::PlayAnimation(const AnimationType& animationtype, bool loopAnim) {
	if (currentAnimPlaying == animationtype) {
		return;
	}

	currentAnimPlaying = animationtype;

	UAnimSequence* AnimationPtr = AnimS->GetAnimation(NpcType, animationtype);
	if (AnimationPtr) {
		SkeletalMesh->PlayAnimation(AnimationPtr, loopAnim);
	}
}

void ABasicNPC::InitializeVisionCollisionSphere(const float& radius) {
	// Creating the collision sphere that detects NPCs
	CollisionNpcDetectionSphere = NewObject<USphereComponent>(this, USphereComponent::StaticClass(), TEXT("CollisionNpcDetectionSphere"));
	CollisionNpcDetectionSphere->InitSphereRadius(radius);
	CollisionNpcDetectionSphere->SetupAttachment(RootComponent);
	CollisionNpcDetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	CollisionNpcDetectionSphere->RegisterComponent();

	// Binding the overlap events for the NPC detection collision sphere
	CollisionNpcDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABasicNPC::OnOverlapBegin);
	CollisionNpcDetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ABasicNPC::OnOverlapEnd);

	// Debugging // TODO Remove this when done debugging 
	CollisionNpcDetectionSphere->SetHiddenInGame(false);
	CollisionNpcDetectionSphere->SetVisibility(WTSR->ShowNpcVisionSpheres);
	CollisionNpcDetectionSphere->SetLineThickness(2.0f);
}

void ABasicNPC::RequestPathToPlayer() {
	FVector npcLocation = GetActorLocation();
	FVector playerLocation = WTSR->getCurrentPlayerPosition();

	PathfindingManager->AddPathfindingTask(this, npcLocation, playerLocation);
}

void ABasicNPC::ConsumePathAndMoveToLocation(const float& DeltaSeconds) {
	// Interpolating smoothly from the current location to the target
	FVector actorLocation = GetActorLocation();

	AdjustRotationTowardsNextLocation(actorLocation, targetLocation, DeltaSeconds);

	// Only jump if there is a difference between the actor and the target
	bool isJumpNeeded = FMath::Abs(actorLocation.Z - targetLocation.Z) > 0.1f;
	if (isJumpNeeded) {
		// Start jump if moving in X/Y direction and not already jumping
		bool isMovingOnXY = FVector::Dist2D(actorLocation, targetLocation) > 10.0f;

		if (!isJumping && isMovingOnXY) {
			isJumping = true;
			jumpProgress = 0.0f;
			jumpStart = actorLocation;
			jumpEnd = targetLocation;
		}
	}

	FVector newPosition;

	if (isJumping) {
		PlayAnimation(AnimationType::Jump);

		// Delay the jump to allow the animation to start first.
		delayJumpCounter += DeltaSeconds;

		if (delayJumpCounter < delayJump) {
			return;
		}

		// Linear interpolation for X and Y movement
		newPosition = FMath::Lerp(jumpStart, jumpEnd, jumpProgress);

		// Parabolic arc for Z movement (ensuring start and end match)
		float midPointZ = FMath::Max(jumpStart.Z, jumpEnd.Z) + jumpHeight;
		newPosition.Z = FMath::Lerp(FMath::Lerp(jumpStart.Z, midPointZ, jumpProgress),
			FMath::Lerp(midPointZ, jumpEnd.Z, jumpProgress),
			jumpProgress);

		jumpProgress += DeltaSeconds * jumpSpeed;

		// End jump when progress reaches 1
		if (jumpProgress >= 1.0f) {
			delayJumpCounter = 0.0f;
			isJumping = false;

			// Ensuring the NPC lands exactly at target
			newPosition = jumpEnd;
		}
	} else {
		PlayAnimation(AnimationType::Walk);
		// Move normally when not jumping
		newPosition = FMath::VInterpConstantTo(actorLocation, targetLocation, DeltaSeconds, DecisionSys->AnimalAttributes.movementSpeed);
	}

	SetActorLocation(newPosition);

	// If the NPC is close enough to the target, consider it reached and move to the next point
	if (FVector::Dist(newPosition, targetLocation) < 1.0f) {
		// Make the current location the target location
		currentLocation = targetLocation;

		targetLocationIsAvailable = false;

		SetTargetLocation();

		// Updating stamina when the NPC reaches a new location (not lower than 0)
		DecisionSys->AnimalAttributes.currentStamina = FMath::Max(
			DecisionSys->AnimalAttributes.currentStamina - DecisionSys->AnimalAttributes.staminaDepletionRate,
			0);
		UpdateStatsVoxelsMesh(StatsType::Stamina);

		// Small chance to make the NPC look around when they reach a target location 
		// to make their actions less robotic
		const float random = FMath::FRand();
		if (random < lookAroundChance) {
			isLookingAround = true;

			// Looking left or right
			if (random < 0.5f) lookingDirection = AnimationType::IdleB;
			else lookingDirection = AnimationType::IdleC;
		}
	}
}

void ABasicNPC::SetTargetLocation() {
	if (InterruptAction) {
		InterruptAction = false;
		SignalEndOfAction();
		return;
	}

	if (pathToTarget->path.empty()) {
		runTargetAnimation = true; // Trigger the final animation
		return;
	}

	// Get the first item in the path
	ActionStatePair* firstItem = pathToTarget->path.front();
	pathToTarget->path.pop_front();

	if (firstItem) {
		targetLocation = firstItem->state->getPosition();

		// Reset the frustration counter when a new target is set
		FrustrationCounter = 0;

		// Return early to prevent a position check if it's the last target and 
		// the action is to attack an NPC (this is to make sure the overlap can
		// happen)
		if (pathToTarget->path.empty() && actionType == ActionType::AttackNpc) {
			targetLocationIsAvailable = true;
			return;
		}

		// Trigger a position check for the new target location
		checkNextPosition = true;
	}
}

bool ABasicNPC::IsTargetLocationAvailable() {
	/*if (this->GetName().Equals("BasicNPC_0")) {  // TODO DELETE THIS AFTER
		UE_LOG(LogTemp, Warning, TEXT("Checking location for BasicNPC_0"));
		if (pathToTarget)  pathToTarget->print();
	}*/

	// Wait longer before checking if the next position is still occupied.
	if (waitForNextPositionCheck) {
		if (OccupiedDelayTimer > OccupiedDelayThreshold) {
			OccupiedDelayTimer = 0.0f;
			waitForNextPositionCheck = false;
		} else {
			float deltaTime = GetWorld()->GetDeltaSeconds();
			OccupiedDelayTimer += deltaTime;
			FrustrationCounter += deltaTime;
			return false;
		}
	}

	bool isLocationOccupied = CLDR->IsLocationOccupied(currentLocation, targetLocation, this);
	if (isLocationOccupied) {
		waitForNextPositionCheck = true;
		return false;
	}

	checkNextPosition = false;
	return true;
}


void ABasicNPC::TimelineProgress(float Value) {
	FVector CurrentPosition = FMath::Lerp(timelineStartPos, timeLineEndPos, Value);
	SetActorLocation(CurrentPosition);
}

void ABasicNPC::SetPathToTargetAndNotify(Path* InPathToTarget) {
	pathToTarget = InPathToTarget;

	if (pathToTarget) {
		SetTargetLocation();
		pathIsReady = true;
	}
}

const AnimalType& ABasicNPC::GetType() {
	return NpcType;
}

const AnimalType& ABasicNPC::GetNpcFoodRelationships() {
	return Relationships.FoodType;
}

const AnimalType& ABasicNPC::GetAlliesRelationships() {
	return Relationships.Allies;
}

const AnimalType& ABasicNPC::GetEnemiesRelationships() {
	return Relationships.Enemies;
}

bool ABasicNPC::IsThreatInRange() {
	return ThreatsInRange.Num() > 0;
}

bool ABasicNPC::IsAllyInRange() {
	return AlliesInRange.Num() > 0;
}

bool ABasicNPC::IsFoodNpcInRange() {  
   return FoodNpcInRange.Num() > 0;  
}

bool ABasicNPC::IsFoodSourceInRange() {
	return FoodSourceInRange.Num() > 0;
}

const FIntPoint& ABasicNPC::GetNpcWorldLocation() {
	return NPCWorldLocation;
}

std::variant<ABasicNPC*, UCustomProceduralMeshComponent*> ABasicNPC::GetClosestInVisionList(VisionList list, bool ChooseOptimalAction, const int& IncrementTargetInVisionList) {
	switch (list) {
	case Threat:
		return GetClosestInList(ThreatsInRange, ChooseOptimalAction, IncrementTargetInVisionList);
		break;
	case Allies:
		return GetClosestInList(AlliesInRange, ChooseOptimalAction, IncrementTargetInVisionList);
		break;
	case NpcFood:
		return GetClosestInList(FoodNpcInRange, ChooseOptimalAction, IncrementTargetInVisionList);
		break;
	case FoodSource:
		return GetClosestInList(FoodSourceInRange, ChooseOptimalAction, IncrementTargetInVisionList);
		break;
	}

	return std::variant<ABasicNPC*, UCustomProceduralMeshComponent*>();
}

FVector& ABasicNPC::GetCurrentLocation() {
	return currentLocation;
}

void ABasicNPC::AttackAndReduceHealth(const int& damage, uint8_t attackerEatingSpeed, ABasicNPC* attacker) {
	const int newHp = DecisionSys->AnimalAttributes.currentHp - damage;
	DecisionSys->AnimalAttributes.currentHp = FMath::Max(newHp, 0);
	UpdateStatsVoxelsMesh(StatsType::HealthPoints);

	if (DecisionSys->AnimalAttributes.currentHp == 0) {
		TriggerNpcDeath(attackerEatingSpeed);
		attacker->TriggerFoodRewardOnKill();
	}
}

bool ABasicNPC::IsDead() {
	return isDeathTriggered;
}

void ABasicNPC::TriggerFoodRewardOnKill() {
	UpdateFoodAttributes(DecisionSys->AnimalAttributes.hungerRecoveryImproved, false);
}

void ABasicNPC::RunTargetAnimationAndUpdateAttributes(float& DeltaSeconds) {
	switch (actionType) {
	case ActionType::AttackNpc:
		// Check if target is still valid
		if (!IsValid(actionTarget)) {
			SignalEndOfAction();
			break;
		}

		// Check if the target is an NPC
		if (ABasicNPC* TargetNPC = Cast<ABasicNPC>(actionTarget)) {
			// Trigger an attack delay based on attack speed (after the first attack) 
			AttackDelayCounter += DeltaSeconds;
			if (delayNextAttack && AttackDelayCounter < DecisionSys->AnimalAttributes.attackSpeed) {
				break;
			}

			// Reset attack delay
			delayNextAttack = false;

			// Attack if the target is close enough and not dead  
			bool isTargetDead = TargetNPC->IsDead();
			if (!isTargetDead && IsTargetLocationCloseEnough(currentLocation, TargetNPC->GetCurrentLocation())) {
				// Reset the counter after the delay is complete  
				AttackDelayCounter = 0.0f;

				PlayAnimation(animationToRunAtTarget);

				// Attack the NPC
				TargetNPC->AttackAndReduceHealth(
					DecisionSys->AnimalAttributes.hitDamage,
					DecisionSys->AnimalAttributes.eatingSpeedRateImproved,
					this);
				delayNextAttack = true;
			}

			// Exit without ending the action, up until the target is despawned
			// This is to make sure the dead NPC will keep getting attacked and
			// not request a new action
			if (isTargetDead) {
				break;
			}
		}

		SignalEndOfAction();
		break;
	case ActionType::AttackFoodSource:
		EatingCounter += DeltaSeconds;

		// Check if the target is still valid and exit early if not
		if (!IsValid(actionTarget)) {
			SignalEndOfAction();
			break;
		}

		PlayAnimation(animationToRunAtTarget);

		if (EatingCounter > DecisionSys->AnimalAttributes.eatingSpeedRateBasic) {
			// Remove the food target and update hunger if the target is valid
			if (IsValid(actionTarget)) {
				// Remove the object when done eating  
				RemoveFoodTargetFromMapAndDestroy();

				// Update the food attributes  
				UpdateFoodAttributes(DecisionSys->AnimalAttributes.hungerRecoveryBasic, true);
			}

			// Trigger end of action and reset counter  
			SignalEndOfAction();
			EatingCounter = 0.0f;
		}
		break;
	case ActionType::RestAfterBasicFood:
		PlayAnimation(animationToRunAtTarget);
		if (UpdateStamina(DeltaSeconds, DecisionSys->AnimalAttributes.restAfterFoodBasic)) {
			// Trigger end of action and reset counter  
			SignalEndOfAction();
			RestCounter = 0.0f;
		}
		break;
	case ActionType::RestAfterImprovedFood:
		PlayAnimation(animationToRunAtTarget);
		if (UpdateStamina(DeltaSeconds, DecisionSys->AnimalAttributes.restAfterFoodImproved)) {
			// Trigger end of action and reset counter  
			SignalEndOfAction();
			RestCounter = 0.0f;
		}
		break;
	case ActionType::TradeFood:
		SignalEndOfAction();
		break;
	case ActionType::Roam:
		SignalEndOfAction();
		break;
	case ActionType::Flee:
		SignalEndOfAction();
		break;
	}

	// TODO Update attributes  

	// TODO Set the runTargetAnimation to false when done  
}

bool ABasicNPC::IsTargetLocationCloseEnough(FVector& current, FVector& target) {
	const float Margin = 30.0f;
	return FMath::Abs(target.X - current.X) <= Margin &&
		FMath::Abs(target.Y - current.Y) <= Margin;
}

// When an action is completed, modify variables to trigger a new action request
void ABasicNPC::SignalEndOfAction() {
	pathIsReady = false;
	pathToTarget = nullptr;
	isTargetSet = false;
	runTargetAnimation = false;
}

void ABasicNPC::AdjustRotationTowardsNextLocation(const FVector& actorLocation, const FVector& targetPosition, const float& deltaTime) {
	// Calculating direction and yaw angle
	FVector direction = (targetPosition - actorLocation).GetSafeNormal();
	float targetYaw = FMath::Atan2(direction.Y, direction.X) * 180.0f / PI;  // Converting radians to degrees

	// Offset for correct NPC direction
	targetYaw -= 90.0f;

	FRotator currentRotation = GetActorRotation();

	// Normalizing the angles manually (wrapping between -180 to 180 degrees)
	float deltaYaw = FMath::Fmod(targetYaw - currentRotation.Yaw + 180.0f, 360.0f) - 180.0f;

	// If the rotation is already close to the target, skip interpolation
	if (FMath::Abs(deltaYaw) > 0.1f) {
		FRotator targetRotation(0.0f, targetYaw, 0.0f);

		// Smoothly interpolating the yaw rotation and apply rotation
		FRotator newRotation = FMath::RInterpTo(currentRotation, targetRotation, deltaTime, rotatingSpeed);
		SetActorRotation(newRotation);
	}
}

void ABasicNPC::RemoveFoodTargetFromMapAndDestroy() {
	// Remove from the spawned map in WTSR
	UCustomProceduralMeshComponent* FoodObject = static_cast<UCustomProceduralMeshComponent*>(actionTarget);
	if (FoodObject->MeshType == MeshType::Flower) {
		WTSR->RemoveSingleFlowerFromMap(FoodObject);
		WTSR->FlowerCount--;
	} else if (FoodObject->MeshType == MeshType::Grass) {
		WTSR->RemoveSingleGrassFromMap(FoodObject);
		WTSR->GrassCount--;
	}

	// Destroy object if it's still valid
	if (IsValid(FoodObject)) {
		FoodObject->UnregisterComponent();
		FoodObject->DestroyComponent();
	}
}

// Update current hunger and food pouch if current hunger reaches the max value
void ABasicNPC::UpdateFoodAttributes(const uint8& hungerRecovered, bool ateBasicFood) {
	if (DecisionSys->AnimalAttributes.currentHunger < DecisionSys->AnimalAttributes.maxHunger) {
		uint8_t updatedHunger = DecisionSys->AnimalAttributes.currentHunger + hungerRecovered;

		if (updatedHunger > DecisionSys->AnimalAttributes.maxHunger) {
			// Add the hunger overflow in the pouch and set current hunger to max
			uint8_t remainder = updatedHunger - DecisionSys->AnimalAttributes.maxHunger;
			DecisionSys->AnimalAttributes.currentHunger = DecisionSys->AnimalAttributes.maxHunger;
			DecisionSys->AnimalAttributes.foodPouch += remainder;

			UpdateStatsVoxelsMesh(StatsType::Hunger);
			UpdateStatsVoxelsMesh(StatsType::FoodPouch);
		} else {
			// Update the current hunger
			DecisionSys->AnimalAttributes.currentHunger = updatedHunger;

			UpdateStatsVoxelsMesh(StatsType::Hunger);
		}
	} else {
		// Add directly to the food pouch 
		DecisionSys->AnimalAttributes.foodPouch += hungerRecovered;
		UpdateStatsVoxelsMesh(StatsType::FoodPouch);
	}

	// Update meals counter
	if (ateBasicFood) {
		DecisionSys->AnimalAttributes.basicMealsCounter += 1;
	} else {
		DecisionSys->AnimalAttributes.improvedMealsCounter += 1;
	}
}

bool ABasicNPC::ForceRestWhenStaminaIsZero(const float& DeltaSeconds) {
	if (DecisionSys->AnimalAttributes.currentStamina == 0) {
		PlayAnimation(AnimationType::Sit);

		bool staminaUpdated = UpdateStamina(DeltaSeconds, DecisionSys->AnimalAttributes.restAfterStaminaIsZero);

		if (staminaUpdated) {
			UpdateStatsVoxelsMesh(StatsType::Stamina);

			// Reset counter and stop resting
			RestCounter = 0.0f;
			return false;
		}

		return true;
	}
	return false;
}

// Update the hunger every unit of time
void ABasicNPC::UpdateHunger(const float& DeltaSeconds) {
	HungerCounter += DeltaSeconds;

	// Decrease by hunger depletation rate (not below zero)
	if (HungerCounter > 1.0f) {
		BasicNpcAttributes& Attributes = DecisionSys->AnimalAttributes;
		int amountNeeded = Attributes.hungerDepletionRate;
		int pouchAvailable = Attributes.foodPouch;
		int fromPouch = FMath::Min(pouchAvailable, amountNeeded);
		Attributes.foodPouch = pouchAvailable - fromPouch;
		amountNeeded = amountNeeded - fromPouch;

		// If there's still some hunger left, take from currentHunger
		if (amountNeeded > 0) {
			int newHunger = static_cast<int>(Attributes.currentHunger) - amountNeeded;
			
			// Clamp at zero and cast back to uint8_t
			Attributes.currentHunger = static_cast<uint8_t>(FMath::Max(newHunger, 0));
		}

		UpdateStatsVoxelsMesh(StatsType::Hunger);
		UpdateStatsVoxelsMesh(StatsType::FoodPouch);

		// Check if NPC should die of starvation
		if (Attributes.currentHunger == 0) {
			TriggerNpcDeath();
		}

		HungerCounter = 0.0f;
	}
}

bool ABasicNPC::UpdateStamina(const float& DeltaSeconds, const uint8_t& Threshold) {
	RestCounter += DeltaSeconds;

	if (RestCounter > Threshold) {
		const uint8_t staminaRecovered = Threshold * DecisionSys->AnimalAttributes.staminaRecoveryRate;
		const uint8_t newStamina = DecisionSys->AnimalAttributes.currentStamina + staminaRecovered;

		// Ensure it doesn't go above max stamina
		DecisionSys->AnimalAttributes.currentStamina = FMath::Max(
			newStamina,
			DecisionSys->AnimalAttributes.maxStamina
		);

		UpdateStatsVoxelsMesh(StatsType::Stamina);

		return true;
	}
	return false;
}


// Run the death animation and set that the NPC should be destroyed
void ABasicNPC::TriggerNpcDeath(uint8_t attackerEatingSpeed) {
	// Set the DespawningTime based on the attacker's eating speed
	DespawnTime = attackerEatingSpeed;

	PlayAnimation(AnimationType::Death, false);
	isDeathTriggered = true;
}

// Wait for the despawn threshold and destroy the NPC
void ABasicNPC::WaitForDespawnThresholdAndDestroy(const float& DeltaSeconds) {
	DespawningCounter += DeltaSeconds;

	if (DespawningCounter >= DespawnTime) {
		WTSR->RemoveSingleNpcFromMap(this);
		this->RemoveFromRoot();
		this->Destroy();
	}
}

void ABasicNPC::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	// Check if the OtherActor is of type ABasicNPC and add to vision list
	if (OtherActor->IsA(ABasicNPC::StaticClass())) {
		// Get type and decide on where to place in the list
		ABasicNPC* OverlappingNPC = Cast<ABasicNPC>(OtherActor);
		if (OverlappingNPC) {
			AddOverlappingNpcToVisionList(OverlappingNPC);
		}

	}

	// Attempt to remove the component from the food source vision list if it's a flower or grass
	if (OtherComp) {
		AddOverlappingBasicFoodSource(OtherComp);
	}
}

void ABasicNPC::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (OtherActor->IsA(ABasicNPC::StaticClass())) {
		// Get type and decide from which vision list to remove it from
		ABasicNPC* OverlappingNPC = Cast<ABasicNPC>(OtherActor);
		if (OverlappingNPC) {
			RemoveOverlappingNpcFromVisionList(OverlappingNPC);
		}
	}

	// Attempt to remove the component from the food source vision list if it's a flower or grass
	if (OtherComp) {
		RemoveOverlappingBasicFoodSource(OtherComp);
	}
}

void ABasicNPC::AddOverlappingNpcToVisionList(ABasicNPC* OverlappingNpc) {
	const AnimalType& OverlappingNpcType = OverlappingNpc->GetType();

	// Check if it's food 
	if ((Relationships.FoodType & OverlappingNpcType) == OverlappingNpcType) {
		FoodNpcInRange.Add(OverlappingNpc);
		return;
	}

	// Check if it's ally
	if ((Relationships.Allies & OverlappingNpcType) == OverlappingNpcType) {
		AlliesInRange.Add(OverlappingNpc);
		return;
	}

	// Check if it's threat
	if ((Relationships.Enemies & OverlappingNpcType) == OverlappingNpcType) {
		ThreatsInRange.Add(OverlappingNpc);
		return;
	}
}

void ABasicNPC::RemoveOverlappingNpcFromVisionList(ABasicNPC* OverlappingNpc) {
	const AnimalType& OverlappingNpcType = OverlappingNpc->GetType();

	// Check if it's food 
	if ((Relationships.FoodType & OverlappingNpcType) == OverlappingNpcType) {
		//UE_LOG(LogTemp, Warning, TEXT("Removed %s from the FOOD NPC vision list."), *OverlappingNpc->GetName());
		FoodNpcInRange.Remove(OverlappingNpc);
		return;
	}

	// Check if it's ally
	if ((Relationships.Allies & OverlappingNpcType) == OverlappingNpcType) {
		//UE_LOG(LogTemp, Warning, TEXT("Removed %s from the ALLIES vision list."), *OverlappingNpc->GetName());
		AlliesInRange.Remove(OverlappingNpc);
		return;
	}

	// Check if it's threat
	if ((Relationships.Enemies & OverlappingNpcType) == OverlappingNpcType) {
		//UE_LOG(LogTemp, Warning, TEXT("Removed %s from the THREATS vision list."), *OverlappingNpc->GetName());
		ThreatsInRange.Remove(OverlappingNpc);
		return;
	}
}

// Add component to food source vision list if it's grass or flower
void ABasicNPC::AddOverlappingBasicFoodSource(UPrimitiveComponent* OverlappingFood) {
	if (UCustomProceduralMeshComponent* CustomMesh = Cast<UCustomProceduralMeshComponent>(OverlappingFood)) {
		if (CustomMesh->MeshType == Flower || CustomMesh->MeshType == Grass) {
			FoodSourceInRange.Add(CustomMesh);
		}
	}
}

// Remoive component from food source vision list if it's grass or flower
void ABasicNPC::RemoveOverlappingBasicFoodSource(UPrimitiveComponent* OverlappingFood) {
	if (UCustomProceduralMeshComponent* CustomMesh = Cast<UCustomProceduralMeshComponent>(OverlappingFood)) {
		if (CustomMesh->MeshType == Flower || CustomMesh->MeshType == Grass) {
			FoodSourceInRange.Remove(CustomMesh);
		}
	}
}

ABasicNPC* ABasicNPC::GetClosestInList(const TArray<ABasicNPC*>& list, bool ChooseOptimalAction, const int& IncrementTargetInVisionList) {
	return GetClosestInListGeneric<ABasicNPC>(list, [](ABasicNPC* npc) -> FVector {
		return npc->GetCurrentLocation();
		}, ChooseOptimalAction, IncrementTargetInVisionList);
}

UCustomProceduralMeshComponent* ABasicNPC::GetClosestInList(const TArray<UCustomProceduralMeshComponent*>& list, bool ChooseOptimalAction, const int& IncrementTargetInVisionList) {
	return GetClosestInListGeneric<UCustomProceduralMeshComponent>(list, [](UCustomProceduralMeshComponent* comp) -> FVector {
		return comp->GetComponentLocation();
		}, ChooseOptimalAction, IncrementTargetInVisionList);
}

// Notify NPCs in the vision list based on the current action 
// (alert allies of enemies, food, or trade)
void ABasicNPC::NotifyNpcsAroundOfEvent(const NpcAction& CurrentAction) {
	UpdateStatsVoxelsMesh(StatsType::Notification, NotificationType::Notifying);
	ShowNotificationStat = true;

	for (ABasicNPC* npc : AlliesInRange) {
		if (IsValid(npc)) {
			npc->ReceiveNotificationOfEvent(CurrentAction);
		}
	}
}

void ABasicNPC::ReceiveNotificationOfEvent(const NpcAction& ActionTriggered) {
	ShowNotificationStat = true;
	if (actionType == ActionTriggered.ActionType) {
		UpdateStatsVoxelsMesh(StatsType::Notification, NotificationType::Discarded);
		return;
	}

	switch(ActionTriggered.ActionType) {
	case ActionType::Flee:
		if (FMath::FRand() < DecisionSys->AnimalAttributes.survivalInstinct) {
			InterruptAction = true;
			UpdateStatsVoxelsMesh(StatsType::Notification, NotificationType::Accepted);
			return;
		}

		break;
	case ActionType::AttackNpc:
		if (FMath::FRand() < DecisionSys->AnimalAttributes.chaseDesire) {
			UpdateStatsVoxelsMesh(StatsType::Notification, NotificationType::Accepted);
			
			// Replace current action and request a path to that location
			ReplaceCurrentActionWithNotifiedAction(ActionTriggered);
			TriggerPathfindingTask();
			return;
		}
		break;
	case ActionType::AttackFoodSource:
		if (AcceptAttackFoodSourceNotification()) {
			UpdateStatsVoxelsMesh(StatsType::Notification, NotificationType::Accepted);

			// Replace current action and request a path to that location
			ReplaceCurrentActionWithNotifiedAction(ActionTriggered);
			TriggerPathfindingTask();
			return;
		}
		break;
	case ActionType::TradeFood:
		break;
	}

	UpdateStatsVoxelsMesh(StatsType::Notification, NotificationType::Discarded);
}

void ABasicNPC::ReplaceCurrentActionWithNotifiedAction(const NpcAction& ActionTriggered) {
	targetLocation = ActionTriggered.TargetLocation;
	animationToRunAtTarget = ActionTriggered.AnimationToRunAtTarget;
	actionType = ActionTriggered.ActionType;
	actionTarget = ActionTriggered.Target;
	isTargetSet = true;
}

bool ABasicNPC::AcceptAttackFoodSourceNotification() {
	// Don't give up chasing an npc for a basic food source
	if (actionType == ActionType::AttackNpc) {
		return false;
	}

	const bool isHungry = DecisionSys->AnimalAttributes.currentHunger > 50;
	const bool wantsToHoard = FMath::FRand() < DecisionSys->AnimalAttributes.desireToHoardFood;
	return isHungry || wantsToHoard;
}

void ABasicNPC::TriggerPathfindingTask() {
	// Adjust location for grass and flower, otherwise the pathfinding will go for the adjacent voxel
	if (actionType == ActionType::AttackFoodSource) {
		targetLocation = FVector(
			targetLocation.X + WTSR->HalfUnrealScale, 
			targetLocation.Y + WTSR->HalfUnrealScale, 
			targetLocation.Z
		);
	}

	PathfindingManager->AddPathfindingTask(this, currentLocation, targetLocation);
}

void ABasicNPC::BeginPlay() {
	Super::BeginPlay();

	spawnNPC();

	AIController = GetWorld()->SpawnActor<AAIController>(AAIController::StaticClass());
	if (AIController) {
		AIController->Possess(this);
	}

	PlayAnimation(AnimationType::IdleA); 
}

void ABasicNPC::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	// PREVENTING THE NPC TO MAKE ANY MOVES FOR THE FIRST N SECONDS
	if (DelayBeforeFirstPathRequest < 1.0f) {
		DelayBeforeFirstPathRequest += DeltaSeconds;
		return;
	}

	// Decrease the hunger every unit of time (1s)
	UpdateHunger(DeltaSeconds);

	if (isDeathTriggered) {
		WaitForDespawnThresholdAndDestroy(DeltaSeconds);
		return;
	}

	if (isLookingAround) {
		PlayAnimation(lookingDirection, false);

		lookingAroundCounter += DeltaSeconds;
		if (lookingAroundCounter >= lookingAroundThreshold) {
			isLookingAround = false;
			lookingAroundCounter = 0;
		} else {
			return;
		}
	}

	// Update the voxel notification stat to default after a certain time
	if (ShowNotificationStat) {
		ShowNotificationStatCounter += DeltaSeconds;
		if (ShowNotificationStatCounter >= ShowNotificationStatThreshold) {
			ShowNotificationStatCounter = 0.0f;
			ShowNotificationStat = false;
			UpdateStatsVoxelsMesh(StatsType::Notification, NotificationType::Default);
		}
	}

	if (pathIsReady) {
		// Rest if there is not enough stamina to move (not in between locations)
		bool isResting = ForceRestWhenStaminaIsZero(DeltaSeconds);
		if (isResting && !targetLocationIsAvailable) return;

		// Making sure the next position is not occupied by another NPC
		if (checkNextPosition || waitForNextPositionCheck) {

			targetLocationIsAvailable = IsTargetLocationAvailable();

			// Terminate the action early if the frustration reaches the threshold
			if (FrustrationCounter >= FrustrationThreshold) {
				frutrationTriggered = true;
				FrustrationCounter = 0.0f;

				// Set the current target, to compare it when a new target is selected
				LastActionTarget = actionTarget;

				SignalEndOfAction();
			}
		}

		// Smoothly move to the next location (happens over multiple frames)
		if (targetLocationIsAvailable) {
			ConsumePathAndMoveToLocation(DeltaSeconds);
		}
	}

	if (!isTargetSet) {
		isTargetSet = true;

		// Request action and set the target
		// If the action is requested based on frustration, a less optimal action is chosen (to avoid the same collision)
		NpcAction NextAction = DecisionSys->GetAction(!frutrationTriggered, sameActionFrustrationCounter);
		targetLocation = NextAction.TargetLocation;
		animationToRunAtTarget = NextAction.AnimationToRunAtTarget;
		actionType = NextAction.ActionType;
		actionTarget = NextAction.Target;

		// Notify NPCs in the vision list 
		if (NextAction.ShouldNotifyOthers) {
			NotifyNpcsAroundOfEvent(NextAction);
		}

		// Increment frustration counter, so that a new target in the vision list can get selected
		// if the same target keeps getting chosen
		if (LastActionTarget == actionTarget && actionTarget != nullptr) {
			sameActionFrustrationCounter++;
		} else {
			sameActionFrustrationCounter = 0;
		}

		// Reset frustration after each action
		frutrationTriggered = false;

		// If resting, avoid triggering a pathfinding request and return early
		bool isResting = actionType == ActionType::RestAfterBasicFood || actionType == ActionType::RestAfterImprovedFood;
		if (isResting) {
			runTargetAnimation = true;
			return;
		}

		TriggerPathfindingTask();
	}

	if (runTargetAnimation) {
		RunTargetAnimationAndUpdateAttributes(DeltaSeconds);
	}
}

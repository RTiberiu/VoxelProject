#include "BasicNPC.h"

#include "..\..\Chunks\TerrainSettings\WorldTerrainSettings.h"
#include "DecisionSystemNPC.h"
#include "KismetProceduralMeshLibrary.h"

#include "GameFramework/PawnMovementComponent.h"

ABasicNPC::ABasicNPC() {
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    RootComponent = SkeletalMesh;
	
    SkeletalMesh->SetCastShadow(false);

    pathToTarget = nullptr;
    pathIsReady = false;
    waitForNextPositionCheck = true;
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

    // TODO I think the DecisionSys might be garbage collected, 
    // since it sometimes has the Owner as a nullptr. This should 
	// never happen since DecisionSys is a member of the ABasicNPC class. 
    // POTENTIAL FIX: I might prevent the garbage collection and destroy it
	// manually when the NPC is destroyed.

    // Create the decision system and initialize it
    DecisionSys = NewObject<UDecisionSystemNPC>();
    DecisionSys->Initialize(this, animalType);

    // Assign the sphere radius from the 
    InitializeVisionCollisionSphere(DecisionSys->AnimalAttributes.awarenessRadius);
}

void ABasicNPC::UpdateStatsVoxelsMesh(StatsType statType) {
    // Get the current and max value for the stat
    float CurrentValue{ 0 };
    float MaxValue{ 0 };

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
    };

    // TODO Calculate the actual fillness value
    const int FillnessValue = GetStatsVoxelNumber(CurrentValue, MaxValue);

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
        Mesh->SetCastShadow(false);
       
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
        SkeletalMesh->SetSkeletalMesh(LoadedMesh);
        SkeletalMesh->SetWorldScale3D(FVector(0.65f, 0.65f, 0.65f)); // TODO I might want to scale this in the editor only once
        
        // Set up simplified collision
        /*SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);
        SkeletalMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);*/
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
    CollisionNpcDetectionSphere->SetVisibility(true);
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
        pathToTarget->path.pop_front();
        currentLocation = targetLocation;

        targetLocationIsAvailable = false;

        SetTargetLocation();

        // Updating stamina when the NPC reaches a new location (not lower than 0)
        DecisionSys->AnimalAttributes.currentStamina = FMath::Max(
            DecisionSys->AnimalAttributes.currentStamina - DecisionSys->AnimalAttributes.staminaDepletionRate,
            0);
        UpdateStatsVoxelsMesh(StatsType::Stamina);
    }
}

void ABasicNPC::SetTargetLocation() {
    if (pathToTarget->path.empty()) {
        runTargetAnimation = true; // Trigger the final animation
        return;
    }

    // Get the first item in the path
    ActionStatePair* firstItem = pathToTarget->path.front();

    if (firstItem) {
        targetLocation = firstItem->state->getPosition();

        // Trigger a position check for the new target location
        waitForNextPositionCheck = true;

        // Make sure the first check happens immediately
        OccupiedDelayTimer = OccupiedDelayThreshold + 1;
    }
}

bool ABasicNPC::IsTargetLocationAvailable() {
    // Wait longer before checking if the next position is still occupied.
    if (waitForNextPositionCheck) {
        if (OccupiedDelayTimer > OccupiedDelayThreshold) {
            OccupiedDelayTimer = 0.0f;
            waitForNextPositionCheck = false;
        } else {
            float deltaTime = GetWorld()->GetDeltaSeconds();
            OccupiedDelayTimer += deltaTime;
            return false;
        }
    }

    bool isLocationOccupied = CLDR->IsLocationOccupied(currentLocation, targetLocation, this);
    if (isLocationOccupied) {
        waitForNextPositionCheck = true;
        return false;
    }

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

std::variant<ABasicNPC*, UCustomProceduralMeshComponent*> ABasicNPC::GetClosestInVisionList(VisionList list) {
    switch (list) {
    case Threat:
        return GetClosestInList(ThreatsInRange);
        break;
    case Allies:
        return GetClosestInList(AlliesInRange);
        break;
    case NpcFood:
        return GetClosestInList(FoodNpcInRange);
        break;
    case FoodSource:
        return GetClosestInList(FoodSourceInRange);
        break;
    }

    return std::variant<ABasicNPC*, UCustomProceduralMeshComponent*>();
}

FVector& ABasicNPC::GetCurrentLocation() {
    return currentLocation;
}

void ABasicNPC::RunTargetAnimationAndUpdateAttributes(float& DeltaSeconds) {
    PlayAnimation(animationToRunAtTarget);

    
    switch (actionType) {
    case ActionType::AttackNpc:
        SignalEndOfAction();
        break;
    case ActionType::AttackFoodSource:
        EatingCounter += DeltaSeconds;

        if (EatingCounter > DecisionSys->AnimalAttributes.eatingSpeedRateBasic) {
            // Remove the object when done eating
            RemoveFoodTargetFromMapAndDestroy();
            
            // Update the food attributes
            UpdateFoodAttributes(DecisionSys->AnimalAttributes.hungerRecoveryBasic, true);

            // Trigger end of action and reset counter
            SignalEndOfAction();
            EatingCounter = 0.0f;
        }
        break;
    case ActionType::RestAfterBasicFood:
        if (UpdateStamina(DeltaSeconds, DecisionSys->AnimalAttributes.restAfterFoodBasic)) {
            // Trigger end of action and reset counter
            SignalEndOfAction();
            RestCounter = 0.0f;
        }

        break;

    case ActionType::RestAfterImprovedFood:
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
    } else if (FoodObject->MeshType == MeshType::Grass) {
        WTSR->RemoveSingleGrassFromMap(FoodObject);
    }

    // TODO Potentially also remove from vision list if it doesn't update automatically
    
    // Destroy object
    FoodObject->UnregisterComponent();
    FoodObject->DestroyComponent();
    WTSR->GrassCount--;
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
        int newHunger = DecisionSys->AnimalAttributes.currentHunger - DecisionSys->AnimalAttributes.hungerDepletionRate;
        DecisionSys->AnimalAttributes.currentHunger = FMath::Max(
            newHunger,
            0
        );

        UpdateStatsVoxelsMesh(StatsType::Hunger);

        // Check if NPC should die of starvation
		if (DecisionSys->AnimalAttributes.currentHunger == 0) {
            // TriggerNpcDeath(); // TODO Uncomment this after done testing, to allow NPCs to die
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
void ABasicNPC::TriggerNpcDeath() {
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
        //UE_LOG(LogTemp, Warning, TEXT("Added %s to the FOOD NPC vision list."), *OverlappingNpc->GetName());
        FoodNpcInRange.Add(OverlappingNpc);
        return;
    }

    // Check if it's ally
    if ((Relationships.Allies & OverlappingNpcType) == OverlappingNpcType) {
        //UE_LOG(LogTemp, Warning, TEXT("Added %s to the ALLIES vision list."), *OverlappingNpc->GetName());
        AlliesInRange.Add(OverlappingNpc);
        return;
    }

    // Check if it's threat
    if ((Relationships.Enemies & OverlappingNpcType) == OverlappingNpcType) {
        //UE_LOG(LogTemp, Warning, TEXT("Added %s to the THREATS vision list."), *OverlappingNpc->GetName());
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

ABasicNPC* ABasicNPC::GetClosestInList(const TArray<ABasicNPC*>& list) {
    return GetClosestInListGeneric<ABasicNPC>(list, [](ABasicNPC* npc) -> FVector {
        return npc->GetCurrentLocation();
        });
}

UCustomProceduralMeshComponent* ABasicNPC::GetClosestInList(const TArray<UCustomProceduralMeshComponent*>& list) {
    return GetClosestInListGeneric<UCustomProceduralMeshComponent>(list, [](UCustomProceduralMeshComponent* comp) -> FVector {
        return comp->GetComponentLocation();
        });
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

    // TESTING ONLY -- PREVENTING THE NPC TO MAKE ANY MOVES FOR THE FIRST N SECONDS
    if (DelayBeforeFirstPathRequest < 2.0f) {
        DelayBeforeFirstPathRequest += DeltaSeconds;
        return;
    }

    // Decrease the hunger every unit of time (1s)
    UpdateHunger(DeltaSeconds);

    if (isDeathTriggered) {
        WaitForDespawnThresholdAndDestroy(DeltaSeconds);
        return;
    }
    
    if (pathIsReady) {
        // Making sure the next position is not occupied by another NPC
        if (waitForNextPositionCheck) {
            targetLocationIsAvailable = IsTargetLocationAvailable();
        }

        // Rest if there is not enough stamina to move
        bool isResting = ForceRestWhenStaminaIsZero(DeltaSeconds);
        if (isResting) return;

        // Smoothly move to the next location (happens over multiple frames)
        if (targetLocationIsAvailable) {
            ConsumePathAndMoveToLocation(DeltaSeconds);
        }
    }

    // TODO IF THE FOOD SOURCE IS GONE, STOP ACTION AND REQUEST NEW ACTION

    if (!isTargetSet) {
        isTargetSet = true;

        // Request action and set the target
        NpcAction NextAction = DecisionSys->GetAction();
        targetLocation = NextAction.TargetLocation;
        animationToRunAtTarget = NextAction.AnimationToRunAtTarget;
        actionType = NextAction.ActionType;
        actionTarget = NextAction.Target;

        // Adjust location for grass and flower, otherwise the pathfinding will go for the adjacent voxel
        if (actionType == ActionType::AttackFoodSource) {
            targetLocation = FVector(targetLocation.X + WTSR->HalfUnrealScale, targetLocation.Y + WTSR->HalfUnrealScale, targetLocation.Z);
        }

        // If resting, avoid triggering a pathfinding request and return early
		bool isResting = actionType == ActionType::RestAfterBasicFood || actionType == ActionType::RestAfterImprovedFood;
		if (isResting) {
            runTargetAnimation = true;
            return;
		}

        PathfindingManager->AddPathfindingTask(this, currentLocation, targetLocation);
    }

    if (runTargetAnimation) {
        RunTargetAnimationAndUpdateAttributes(DeltaSeconds);
    }
}

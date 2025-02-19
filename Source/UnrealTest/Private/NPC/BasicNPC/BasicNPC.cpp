#include "BasicNPC.h"

#include "..\..\Chunks\TerrainSettings\WorldTerrainSettings.h"

#include "GameFramework/PawnMovementComponent.h"

ABasicNPC::ABasicNPC() {
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    RootComponent = SkeletalMesh;
	
    SkeletalMesh->SetCastShadow(false);

    pathToPlayer = nullptr;
    pathIsReady = false;
    waitForNextPositionCheck = false;
    targetLocationIsAvailable = false;
    targetLocation = nullptr;

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

void ABasicNPC::InitializeBrain(const AnimalType& animalType) {
    NpcType = animalType;
    Relationships = AnimalsRelationships[NpcType];

    // Create the decision system and initialize it
    DecisionSys = NewObject<UDecisionSystemNPC>();
    DecisionSys->Initialize(this, animalType);

    // Assign the sphere radius from the 
    InitializeVisionCollisionSphere(DecisionSys->AnimalAttributes.awarenessRadius);
}

void ABasicNPC::spawnNPC() {
    FString MeshPath = AnimS->GetSkeletalMeshPath(NpcType);;
    
    // Load the skeletal mesh
    USkeletalMesh* LoadedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
    if (LoadedMesh) {
        SkeletalMesh->SetSkeletalMesh(LoadedMesh);
        SkeletalMesh->SetWorldScale3D(FVector(0.65f, 0.65f, 0.65f)); // TODO I might want to scale this in the editor only once
        
        // Set up simplified collision
        /*SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Block);
        SkeletalMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);*/
    }

    // Load the animation asset
    PlayAnimation(TEXT("idleA"));

    currentLocation = GetActorLocation();
}

void ABasicNPC::PlayAnimation(const FString& animationtype) {
    if (currentAnimPlaying == animationtype) {
        return;
    }

    currentAnimPlaying = animationtype;

    
    UAnimSequence* AnimationPtr = AnimS->GetAnimation(NpcType, animationtype);
    if (AnimationPtr) {
        SkeletalMesh->PlayAnimation(AnimationPtr, true);
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
}

void ABasicNPC::RequestPathToPlayer() {
    FVector npcLocation = GetActorLocation();
	FVector playerLocation = WTSR->getCurrentPlayerPosition();

    PathfindingManager->AddPathfindingTask(this, npcLocation, playerLocation);
}

void ABasicNPC::ConsumePathAndMoveToLocation() {
    float deltaTime = GetWorld()->GetDeltaSeconds();

    // Interpolating smoothly from the current location to the target
    FVector actorLocation = GetActorLocation();

    AdjustRotationTowardsNextLocation(actorLocation, *targetLocation, deltaTime);

    // Only jump if there is a difference between the actor and the target
    bool isJumpNeeded = FMath::Abs(actorLocation.Z - targetLocation->Z) > 5.0f;
    if (isJumpNeeded) {
        // Start jump if moving in X/Y direction and not already jumping
        bool isMovingOnXY = FVector::Dist2D(actorLocation, *targetLocation) > 10.0f;

        if (!isJumping && isMovingOnXY) {
            isJumping = true;
            jumpProgress = 0.0f;
            jumpStart = actorLocation;
            jumpEnd = *targetLocation;
        }
    }

    FVector newPosition;

    if (isJumping) {
        PlayAnimation(TEXT("jump"));
        // Linear interpolation for X and Y movement
        newPosition = FMath::Lerp(jumpStart, jumpEnd, jumpProgress);

        // Parabolic arc for Z movement (ensuring start and end match)
        float midPointZ = FMath::Max(jumpStart.Z, jumpEnd.Z) + jumpHeight;
        newPosition.Z = FMath::Lerp(FMath::Lerp(jumpStart.Z, midPointZ, jumpProgress),
            FMath::Lerp(midPointZ, jumpEnd.Z, jumpProgress),
            jumpProgress);

        jumpProgress += deltaTime * jumpSpeed;

        // End jump when progress reaches 1
        if (jumpProgress >= 1.0f) {
            isJumping = false;
            // Ensuring the NPC lands exactly at target
            newPosition = jumpEnd; 
        }
    } else {
        PlayAnimation(TEXT("walk"));
        // Move normally when not jumping
        newPosition = FMath::VInterpTo(actorLocation, *targetLocation, deltaTime, DecisionSys->AnimalAttributes.movementSpeed);
    }

    SetActorLocation(newPosition);

    // If the NPC is close enough to the target, consider it reached and move to the next point
    if (FVector::Dist(newPosition, *targetLocation) < 1.0f) {
        pathToPlayer->path.pop_front();
        currentLocation = *targetLocation;

        targetLocationIsAvailable = false;

        SetTargetLocation();
    }
}

void ABasicNPC::SetTargetLocation() {
    if (pathToPlayer->path.empty()) {
        pathIsReady = false;
        pathToPlayer = nullptr;
        targetLocation = nullptr;
        return;
    }

    // Get the first item in the path
    ActionStatePair* firstItem = pathToPlayer->path.front();

    if (firstItem) {
        targetLocation = &firstItem->state->getPosition();

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

    bool isLocationOccupied = CLDR->IsLocationOccupied(currentLocation, *targetLocation, this);
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

void ABasicNPC::SetPathToPlayerAndNotify(Path* InPathToPlayer) {
	pathToPlayer = InPathToPlayer;

    if (pathToPlayer) {
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
    if (FMath::Abs(deltaYaw) > 1.0f) {
        FRotator targetRotation(0.0f, targetYaw, 0.0f);

        // Smoothly interpolating the yaw rotation and apply rotation
        FRotator newRotation = FMath::RInterpTo(currentRotation, targetRotation, deltaTime, DecisionSys->AnimalAttributes.movementSpeed);
        SetActorRotation(newRotation);
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
        UE_LOG(LogTemp, Warning, TEXT("Added %s to the FOOD NPC vision list."), *OverlappingNpc->GetName())
        FoodNpcInRange.Add(OverlappingNpc);
        return;
    }

    // Check if it's ally
    if ((Relationships.Allies & OverlappingNpcType) == OverlappingNpcType) {
        UE_LOG(LogTemp, Warning, TEXT("Added %s to the ALLIES vision list."), *OverlappingNpc->GetName())
        AlliesInRange.Add(OverlappingNpc);
        return;
    }

    // Check if it's threat
    if ((Relationships.Enemies & OverlappingNpcType) == OverlappingNpcType) {
        UE_LOG(LogTemp, Warning, TEXT("Added %s to the THREATS vision list."), *OverlappingNpc->GetName())
        ThreatsInRange.Add(OverlappingNpc);
        return;
    }
}

void ABasicNPC::RemoveOverlappingNpcFromVisionList(ABasicNPC* OverlappingNpc) {
    const AnimalType& OverlappingNpcType = OverlappingNpc->GetType();

    // Check if it's food 
    if ((Relationships.FoodType & OverlappingNpcType) == OverlappingNpcType) {
        UE_LOG(LogTemp, Warning, TEXT("Removed %s from the FOOD NPC vision list."), *OverlappingNpc->GetName())
        FoodNpcInRange.Remove(OverlappingNpc);
        return;
    }

    // Check if it's ally
    if ((Relationships.Allies & OverlappingNpcType) == OverlappingNpcType) {
        UE_LOG(LogTemp, Warning, TEXT("Removed %s from the ALLIES vision list."), *OverlappingNpc->GetName())
        AlliesInRange.Remove(OverlappingNpc);
        return;
    }

    // Check if it's threat
    if ((Relationships.Enemies & OverlappingNpcType) == OverlappingNpcType) {
        UE_LOG(LogTemp, Warning, TEXT("Removed %s from the THREATS vision list."), *OverlappingNpc->GetName())
        ThreatsInRange.Remove(OverlappingNpc);
        return;
    }
}

// Add component to food source vision list if it's grass or flower
void ABasicNPC::AddOverlappingBasicFoodSource(UPrimitiveComponent* OverlappingFood) {
    if (OverlappingFood->ComponentTags.Contains(TEXT("Grass"))) {
        UE_LOG(LogTemp, Warning, TEXT("Added GRASS to FOOD SOURCE vision list."));
        FoodSourceInRange.Add(OverlappingFood);
        return;
    }

    if (OverlappingFood->ComponentTags.Contains(TEXT("Flower"))) {
        UE_LOG(LogTemp, Warning, TEXT("Added FLOWER to FOOD SOURCE vision list."));
        FoodSourceInRange.Add(OverlappingFood);
        return;
    }
}

// Remoive component from food source vision list if it's grass or flower
void ABasicNPC::RemoveOverlappingBasicFoodSource(UPrimitiveComponent* OverlappingFood) {
    if (OverlappingFood->ComponentTags.Contains(TEXT("Grass"))) {
        UE_LOG(LogTemp, Warning, TEXT("Removed GRASS to FOOD SOURCE vision list."));
        FoodSourceInRange.Remove(OverlappingFood);
        return;
    }

    if (OverlappingFood->ComponentTags.Contains(TEXT("Flower"))) {
        UE_LOG(LogTemp, Warning, TEXT("Removed FLOWER to FOOD SOURCE vision list."));
        FoodSourceInRange.Remove(OverlappingFood);
        return;
    }
}

void ABasicNPC::BeginPlay() {
	Super::BeginPlay();

    spawnNPC();

    AIController = GetWorld()->SpawnActor<AAIController>(AAIController::StaticClass());
    if (AIController) {
        AIController->Possess(this);
    }

    PlayAnimation(TEXT("idleA"));
}

void ABasicNPC::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    DelayBeforeFirstPathRequest += DeltaSeconds;

    if (DelayBeforeFirstPathRequest < 8.0f) {
        return;
    }

    if (pathToPlayer == nullptr) {
        PlayAnimation(TEXT("idleA"));

        RequestPathToPlayer();
    }

    TimeSinceLastCall += DeltaSeconds;

    if (pathIsReady) {
        // Making sure the next position is not occupied by another NPC
        if (waitForNextPositionCheck) {
            targetLocationIsAvailable = IsTargetLocationAvailable();
        }

        // Smoothly move to the next location (happens over multiple frames)
        if (targetLocationIsAvailable) {
            ConsumePathAndMoveToLocation();
        }
    }
}

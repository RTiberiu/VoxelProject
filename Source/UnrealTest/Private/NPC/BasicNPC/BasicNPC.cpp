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

    UPawnMovementComponent* MovementComponent = FindComponentByClass<UPawnMovementComponent>();
    if (!MovementComponent) {
        FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
    }

    movementSpeed = 8.0f;
}

ABasicNPC::~ABasicNPC() {
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

void ABasicNPC::spawnNPC() {
    FString MeshPath = TEXT("SkeletalMesh'/Game/Characters/Animals/Panda/Animations/Panda_Animations.Panda_Animations'");

    // Load the skeletal mesh
    USkeletalMesh* LoadedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
    if (LoadedMesh) {
        SkeletalMesh->SetSkeletalMesh(LoadedMesh);
        SkeletalMesh->SetWorldScale3D(FVector(0.65f, 0.65f, 0.65f)); // TODO I might want to scale this in the editor only once
    }

    // Load the animation asset
    PlayAnimation(TEXT("idleA"));

    currentLocation = GetActorLocation();
}

void ABasicNPC::buildAnimationsList() {
    const TArray<FString> AnimationNames = {
        "AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Idle_A.Panda_Animations_Anim_Idle_A'",
        "AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Bounce.Panda_Animations_Anim_Bounce'",
        "AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Clicked.Panda_Animations_Anim_Clicked'",
        "AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Death.Panda_Animations_Anim_Death'",
        "AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Eat.Panda_Animations_Anim_Eat'",
        "AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Fly.Panda_Animations_Anim_Fly'",
        "AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Walk.Panda_Animations_Anim_Walk'",
        "AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Jump.Panda_Animations_Anim_Jump'"
    };

    const TArray<FString> AnimationKeys = {
        "idleA",
        "bounce",
        "clicked",
        "death",
        "eat",
        "fly",
        "walk",
        "jump"
    };

    for (int x = 0; x < AnimationNames.Num(); x++) {
        UAnimSequence* LoadedAnim = LoadObject<UAnimSequence>(nullptr, *AnimationNames[x]);
        Animations.Add(AnimationKeys[x], LoadedAnim);
    }
}

void ABasicNPC::PlayAnimation(const FString& type) {
    if (currentAnimPlaying == type) {
        return;
    }

    currentAnimPlaying = type;

    UAnimSequence** AnimationPtr = Animations.Find(type);

    if (AnimationPtr) {
        SkeletalMesh->PlayAnimation(*AnimationPtr, true);
    }
}

void ABasicNPC::RequestPathToPlayer() {
    FVector npcLocation = GetActorLocation();
	FVector playerLocation = WTSR->getCurrentPlayerPosition();

    PathfindingManager->AddPathfindingTask(this, npcLocation, playerLocation);
}

void ABasicNPC::ConsumePathAndMoveToLocation() {
    if (pathToPlayer->path.empty()) {
        pathToPlayer = nullptr;
        pathIsReady = false;
        return;
    }

    // Get the first item
    ActionStatePair* firstItem = pathToPlayer->path.front();
    FVector targetPosition = firstItem->state->getPosition();

    // Interpolate smoothly from the current location to the target using VInterpTo
    float deltaTime = GetWorld()->GetDeltaSeconds();
    FVector actorLocation = GetActorLocation();

    // Only jump if there is a difference between the actor and the target
    bool isJumpNeeded = FMath::Abs(actorLocation.Z - targetPosition.Z) > 5.0f;
    if (isJumpNeeded) {
        // Start jump if moving in X/Y direction and not already jumping
        bool isMovingOnXY = FVector::Dist2D(actorLocation, targetPosition) > 10.0f;

        if (!isJumping && isMovingOnXY) {
            isJumping = true;
            jumpProgress = 0.0f;
            jumpStart = actorLocation;
            jumpEnd = targetPosition;
        }
    }

    FVector newPosition;

    if (isJumping) {
        PlayAnimation(TEXT("jump"));
        // Linear interpolation for X & Y movement
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
        newPosition = FMath::VInterpTo(actorLocation, targetPosition, deltaTime, movementSpeed);
    }

    SetActorLocation(newPosition);

    // If the NPC is close enough to the target, consider it reached and move to the next point
    if (FVector::Dist(newPosition, targetPosition) < 10.0f) {
        pathToPlayer->path.pop_front();
        currentLocation = targetPosition;
    }
}

void ABasicNPC::TimelineProgress(float Value) {
    FVector CurrentPosition = FMath::Lerp(timelineStartPos, timeLineEndPos, Value);
    SetActorLocation(CurrentPosition);
}

void ABasicNPC::SetPathToPlayerAndNotify(Path* InPathToPlayer) {
	pathToPlayer = InPathToPlayer;
    pathIsReady = true;
}

void ABasicNPC::BeginPlay() {
	Super::BeginPlay();

    buildAnimationsList();
	
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
        ConsumePathAndMoveToLocation();
    }
}

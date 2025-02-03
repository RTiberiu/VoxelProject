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
    UAnimSequence* LoadedAnim = LoadObject<UAnimSequence>(nullptr, *Animations[0]);
    if (LoadedAnim) {
        SkeletalMesh->PlayAnimation(LoadedAnim, true);
    }

    currentLocation = GetActorLocation();
}

void ABasicNPC::buildAnimationsList() {
    Animations.Emplace(FString("AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Idle_A.Panda_Animations_Anim_Idle_A'"));
    Animations.Emplace(FString("AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Bounce.Panda_Animations_Anim_Bounce'"));
    Animations.Emplace(FString("AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Clicked.Panda_Animations_Anim_Clicked'"));
    Animations.Emplace(FString("AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Death.Panda_Animations_Anim_Death'"));
    Animations.Emplace(FString("AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Eat.Panda_Animations_Anim_Eat'"));
    Animations.Emplace(FString("AnimSequence'/Game/Characters/Animals/Panda/Animations/Panda_Animations_Anim_Fly.Panda_Animations_Anim_Fly'"));
}

void ABasicNPC::PlayRandomAnimation() {
    // Play a new random animation
    if (animationFrameCounter >= animationChangeAfterFrames) {

        FString& Animation = Animations[FMath::RandRange(0, Animations.Num() - 1)];

        UAnimSequence* LoadedAnim = LoadObject<UAnimSequence>(nullptr, *Animation);
        if (LoadedAnim) {
            SkeletalMesh->PlayAnimation(LoadedAnim, true);
        }

        animationFrameCounter = 0;
    }

    animationFrameCounter++;
}

void ABasicNPC::RequestPathToPlayer() {
    FVector npcLocation = GetActorLocation();
	FVector playerLocation = WTSR->getCurrentPlayerPosition();

    PathfindingManager->AddPathfindingTask(this, npcLocation, playerLocation);
}

void ABasicNPC::ConsumePathAndMoveToLocation() {
    if (pathToPlayer && !pathToPlayer->path.empty()) {
        // Get the first item
        ActionStatePair* firstItem = pathToPlayer->path.front();
        FVector targetPosition = firstItem->state->getPosition();

        // Interpolate smoothly from the current location to the target using VInterpTo
        float deltaTime = GetWorld()->GetDeltaSeconds();
        FVector newPosition = FMath::VInterpTo(GetActorLocation(), targetPosition, deltaTime, movementSpeed);
        SetActorLocation(newPosition);

        // If the NPC is close enough to the target, consider it reached and move to the next point
        if (FVector::Dist(newPosition, targetPosition) < 10.0f) {
            pathToPlayer->path.pop_front();
            currentLocation = targetPosition;
        }
    } else {
        pathToPlayer = nullptr;
        pathIsReady = false;
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

    PlayRandomAnimation();
}

void ABasicNPC::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    DelayBeforeFirstPathRequest += DeltaSeconds;

    if (DelayBeforeFirstPathRequest < 8.0f) {
        return;
    }

    if (pathToPlayer == nullptr) {
        RequestPathToPlayer();
    }

    TimeSinceLastCall += DeltaSeconds;

    if (pathIsReady) {
        ConsumePathAndMoveToLocation();
    }
}

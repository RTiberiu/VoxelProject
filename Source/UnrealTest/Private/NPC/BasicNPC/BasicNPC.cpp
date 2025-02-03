#include "BasicNPC.h"

#include <Runtime/AIModule/Classes/AIController.h>
#include "..\..\Chunks\TerrainSettings\WorldTerrainSettings.h"

#include "GameFramework/PawnMovementComponent.h"

ABasicNPC::ABasicNPC() {
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    RootComponent = SkeletalMesh;
	
    SkeletalMesh->SetCastShadow(false);

    // SkeletalMesh->SetupAttachment(RootComponent);

    pathToPlayer = nullptr;
    pathIsReady = false;

    UPawnMovementComponent* MovementComponent = FindComponentByClass<UPawnMovementComponent>();
    if (!MovementComponent) {
        FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
    }
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
    // TODO IMPLEMENET MOVING TO EACH VECTOR AND ANIMATE AT EACH POINT
    // UE_LOG(LogTemp, Warning, TEXT("Consuming path and moving to location")); 

    if (!pathToPlayer->path.empty()) {
        // Get the first item
        ActionStatePair* firstItem = pathToPlayer->path.front();

        // Remove the first item from the list
        pathToPlayer->path.pop_front();

        SetActorLocation(firstItem->state->getPosition());
    } else {

        // TESTING NOT RESETTING (Meaning no request for another path)
        pathToPlayer = nullptr;
        pathIsReady = false;
    }
}

void ABasicNPC::SetPathToPlayerAndNotify(Path* InPathToPlayer) {
	pathToPlayer = InPathToPlayer;
    pathIsReady = true;
}

void ABasicNPC::BeginPlay() {
	Super::BeginPlay();

    buildAnimationsList();
	
    spawnNPC();

    AAIController* AIController = GetWorld()->SpawnActor<AAIController>(AAIController::StaticClass());
    if (AIController) {
        AIController->Possess(this);
    }

    PlayRandomAnimation();
}

void ABasicNPC::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    // PlayRandomAnimation();

    DelayBeforeFirstPathRequest += DeltaSeconds;

    if (DelayBeforeFirstPathRequest < 12.0f) {
        return;
    }

    if (pathToPlayer == nullptr) {
        RequestPathToPlayer();
    }

    TimeSinceLastCall += DeltaSeconds;

    if (TimeSinceLastCall >= 0.2f) {
        // Move the player to the location if path is ready
        if (pathIsReady) {
            ConsumePathAndMoveToLocation();
        }

        TimeSinceLastCall = 0.0f;
    }
}

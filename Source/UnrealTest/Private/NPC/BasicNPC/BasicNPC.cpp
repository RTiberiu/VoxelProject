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

void ABasicNPC::GetPathToPlayer() {
    FVector npcLocation = GetActorLocation();
	FVector playerLocation = WTSR->getCurrentPlayerPosition();

    // TODO NEED TO PASS HERE A REFERENCE TO THE CURRENT NPC OBJECT
    // AND LET THE TASK NOTIFY IT WHEN THE PATH IS AVAILABLE
    // 
	//      TODO Modify the task to receive a pointer to the current NPC object
	//      TODO Call a function in the NPC object to notify it when the path is available
	//      TODO The NPC object will then call a function in the AIController to move the NPC along the path
    PathfindingManager->AddPathfindingTask(npcLocation, playerLocation);
}

void ABasicNPC::BeginPlay() {
	Super::BeginPlay();

    buildAnimationsList();
	
    spawnNPC();

    AAIController* AIController = GetWorld()->SpawnActor<AAIController>(AAIController::StaticClass());
    if (AIController) {
        AIController->Possess(this);
    }



}

void ABasicNPC::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

    PlayRandomAnimation();

}

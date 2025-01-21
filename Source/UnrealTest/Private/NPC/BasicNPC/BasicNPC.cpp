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


    UPawnMovementComponent* MovementComponent = FindComponentByClass<UPawnMovementComponent>();
    if (!MovementComponent) {
        // UE_LOG(LogTemp, Error, TEXT("Pawn lacks a compatible movement component!"));

        // TODO Continue from here to allow the NPC to move on the navmesh
        // 1. Move the mesh in the center of the player (-X and -Z of the navmesh volume from the player's location)
        // 2. Research how can I make the navmesh actually go on my terrain, as currently a lot of the voxels 
        // are not actually used by the navmesh. Inspect by opening the command ~ or ` and running the command:
        // show navigation
        FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
    }
}

ABasicNPC::~ABasicNPC() {
}

void ABasicNPC::SetNPCWorldLocation(FIntPoint InTreeLocation) {
	NPCLocation = InTreeLocation;
}

void ABasicNPC::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
    WorldTerrainSettingsRef = InWorldTerrainSettings;
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

    // Play a new random animation
    if (frameCounter >= animationChangeAfterFrames) {
        
        FString& Animation = Animations[FMath::RandRange(0, Animations.Num() - 1)];

        UAnimSequence* LoadedAnim = LoadObject<UAnimSequence>(nullptr, *Animation);
        if (LoadedAnim) {
            SkeletalMesh->PlayAnimation(LoadedAnim, true);
        }

        frameCounter = 0;
    }

    frameCounter++;
}

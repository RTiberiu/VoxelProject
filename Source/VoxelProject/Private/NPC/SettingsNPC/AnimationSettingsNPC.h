#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RelationshipSettingsNPC.h"
#include "AnimationSettingsNPC.generated.h"

enum class AnimationType {
	IdleA,
	IdleB,
	IdleC,
	Bounce,
	Clicked,
	Death,
	Eat,
	Fly,
	Walk,
	Jump,
	Run,
	Sit,
	Hit,
	Spin,
	Attack
};

UCLASS()
class UAnimationSettingsNPC : public UObject {
	GENERATED_BODY()

public:

	UAnimationSettingsNPC();
	~UAnimationSettingsNPC();

	UAnimSequence* GetAnimation(const AnimalType& animal, const AnimationType& animationType);

	FString GetSkeletalMeshPath(const AnimalType& animal);

private:
	void LoadAnimationsForAllAnimals();

	TMap<AnimationType, UAnimSequence*>* GetAnimalAnimationList(const AnimalType& animal);

	const TArray<AnimalType> Animals = {
			AnimalType::Tiger,
			AnimalType::Tapir,
			AnimalType::Gorilla,
			AnimalType::Bat,
			AnimalType::Sloth,
			AnimalType::Cobra,
			AnimalType::Peacock,
			AnimalType::Panda,
			AnimalType::RedPanda
	};

	const FString BaseSkeletalMeshPath = "SkeletalMesh'/Game/Characters/Animals/";

    const TMap<AnimalType, FString> SkeletalPath = {
		{AnimalType::Tiger,   "Tiger/Tiger_Animations.Tiger_Animations'"},
		{AnimalType::Tapir,   "Tapir/Tapir_Animations.Tapir_Animations'"},
		{AnimalType::Gorilla, "Gorilla/Gorilla_Animations.Gorilla_Animations'"},
		{AnimalType::Bat,     "Bat/Bat_Animations.Bat_Animations'"},
		{AnimalType::Sloth,   "Sloth/Sloth_Animations.Sloth_Animations'"},
		{AnimalType::Cobra,   "Cobra/Cobra_Animations.Cobra_Animations'"},
		{AnimalType::Peacock, "Peacock/Peacock_Animations.Peacock_Animations'"},
		{AnimalType::Panda,   "Panda/Panda_Animations.Panda_Animations'"},
		{AnimalType::RedPanda,   "RedPanda/RedPanda_Animations.RedPanda_Animations'"}
    };
	
	// Every other animation path for an NPC should follow this structure 
	const TArray<AnimationType> AnimationKeys = {
		AnimationType::IdleA,
		AnimationType::IdleB,
		AnimationType::IdleC,
		AnimationType::Bounce,
		AnimationType::Clicked,
		AnimationType::Death,
		AnimationType::Eat,
		AnimationType::Fly,
		AnimationType::Walk,
		AnimationType::Jump,
		AnimationType::Run,
		AnimationType::Sit,
		AnimationType::Hit,
		AnimationType::Spin,
		AnimationType::Attack
	};
	
	const FString BaseAnimationPath = "/Script/Engine.AnimSequence'/Game/Characters/Animals/";

	const TArray<FString> AnimationNamesPath = {
			"/Idle_A.Idle_A'",
			"/Idle_B.Idle_B'",
			"/Idle_C.Idle_C'",
			"/Bounce.Bounce'",
			"/Clicked.Clicked'",
			"/Death.Death'",
			"/Eat.Eat'",
			"/Fly.Fly'",
			"/Walk.Walk'",
			"/Jump.Jump'",
			"/Run.Run'",
			"/Sit.Sit'",
			"/Hit.Hit'",
			"/Spin.Spin'",
			"/Attack.Attack'"
	};

	TMap<AnimationType, UAnimSequence*> TigerLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> TapirLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> GorillaLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> BatLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> SlothLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> CobraLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> PeacockLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> PandaLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> RedPandaLoadedAnimations;

	TMap<AnimalType, TMap<AnimationType, UAnimSequence*>*> AnimalAnimationMap = {
		{AnimalType::Tiger,   &TigerLoadedAnimations},
		{AnimalType::Tapir,   &TapirLoadedAnimations},
		{AnimalType::Gorilla, &GorillaLoadedAnimations},
		{AnimalType::Bat,     &BatLoadedAnimations},
		{AnimalType::Sloth,   &SlothLoadedAnimations},
		{AnimalType::Cobra,   &CobraLoadedAnimations},
		{AnimalType::Peacock, &PeacockLoadedAnimations},
		{AnimalType::Panda,   &PandaLoadedAnimations},
		{AnimalType::RedPanda,   &RedPandaLoadedAnimations},
	};
};



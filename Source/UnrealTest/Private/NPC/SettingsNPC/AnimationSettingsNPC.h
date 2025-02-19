#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RelationshipSettingsNPC.h"
#include "AnimationSettingsNPC.generated.h"

// Setting the bit for each of the type of animal
enum class AnimationType : uint16_t {
	IdleA = 1 << 0,
	Bounce = 1 << 1,
	Clicked = 1 << 2,
	Death = 1 << 3,
	Eat = 1 << 4,
	Fly = 1 << 5,
	Walk = 1 << 6,
	Jump = 1 << 7,
	Run = 1 << 8,
	Sit = 1 << 9,
	Hit = 1 << 10,
	Spin = 1 << 11,
	Attack = 1 << 12
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
			AnimalType::Panda
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
		{AnimalType::Panda,   "Panda/Panda_Animations.Panda_Animations'"}
    };

	// Every other animation path for an NPC should follow this structure 
	const TArray<AnimationType> AnimationKeys = {
		AnimationType::IdleA,
		AnimationType::Bounce,
		AnimationType::Clicked,
		AnimationType::Death,
		AnimationType::Eat,
		AnimationType::Fly,
		AnimationType::Walk,
		AnimationType::Jump,
		AnimationType::Run
	};
	
	const FString BaseAnimationPath = "/Script/Engine.AnimSequence'/Game/Characters/Animals/";

	const TArray<FString> AnimationNamesPath = {
			"/Idle_A.Idle_A'",
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
			"/Attack.Attack'",
	};

	TMap<AnimationType, UAnimSequence*> TigerLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> TapirLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> GorillaLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> BatLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> SlothLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> CobraLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> PeacockLoadedAnimations;
	TMap<AnimationType, UAnimSequence*> PandaLoadedAnimations;

	TMap<AnimalType, TMap<AnimationType, UAnimSequence*>*> AnimalAnimationMap = {
		{AnimalType::Tiger,   &TigerLoadedAnimations},
		{AnimalType::Tapir,   &TapirLoadedAnimations},
		{AnimalType::Gorilla, &GorillaLoadedAnimations},
		{AnimalType::Bat,     &BatLoadedAnimations},
		{AnimalType::Sloth,   &SlothLoadedAnimations},
		{AnimalType::Cobra,   &CobraLoadedAnimations},
		{AnimalType::Peacock, &PeacockLoadedAnimations},
		{AnimalType::Panda,   &PandaLoadedAnimations}
	};
};



#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RelationshipSettingsNPC.h"
#include "AnimationSettingsNPC.generated.h"

UCLASS()
class UAnimationSettingsNPC : public UObject {
	GENERATED_BODY()

public:

	UAnimationSettingsNPC();
	~UAnimationSettingsNPC();

	UAnimSequence* GetAnimation(const AnimalType& animal, const FString& animationType);

	FString GetSkeletalMeshPath(const AnimalType& animal);

private:
	void LoadAnimationsForAllAnimals();

	TMap<FString, UAnimSequence*>* GetAnimalAnimationList(const AnimalType& animal);

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
	const TArray<FString> AnimationKeys = {
		"idleA",
		"bounce",
		"clicked",
		"death",
		"eat",
		"fly",
		"walk",
		"jump",
		"run"
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
	};

	TMap<FString, UAnimSequence*> TigerLoadedAnimations;
	TMap<FString, UAnimSequence*> TapirLoadedAnimations;
	TMap<FString, UAnimSequence*> GorillaLoadedAnimations;
	TMap<FString, UAnimSequence*> BatLoadedAnimations;
	TMap<FString, UAnimSequence*> SlothLoadedAnimations;
	TMap<FString, UAnimSequence*> CobraLoadedAnimations;
	TMap<FString, UAnimSequence*> PeacockLoadedAnimations;
	TMap<FString, UAnimSequence*> PandaLoadedAnimations;

	TMap<AnimalType, TMap<FString, UAnimSequence*>*> AnimalAnimationMap = {
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



#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationSettingsNPC.generated.h"

UCLASS()
class UAnimationSettingsNPC : public UObject {
	GENERATED_BODY()

public:

	UAnimationSettingsNPC();
	~UAnimationSettingsNPC();

	UAnimSequence* GetAnimation(const FString& animal, const FString& animationType);

	FString GetSkeletalMeshPath(const FString& animal);

private:
	void LoadAnimationsForAllAnimals();

	TMap<FString, UAnimSequence*>* GetAnimalAnimationList(const FString& animal);

	const TArray<FString> Animals = {
		"Tiger", "Tapir", "Gorilla", "Bat", "Sloth", "Cobra", "Peacock", "Panda"
	};

	const FString BaseSkeletalMeshPath = "SkeletalMesh'/Game/Characters/Animals/";

    const TMap<FString, FString> SkeletalPath = {
        {"Tiger", "Tiger/Tiger_Animations.Tiger_Animations'"},
		{ "Tapir", "Tapir/Tapir_Animations.Tapir_Animations'" },
		{ "Gorilla", "Gorilla/Gorilla_Animations.Gorilla_Animations'" },
		{ "Bat", "Bat/Bat_Animations.Bat_Animations'" },
		{ "Sloth", "Sloth/Sloth_Animations.Sloth_Animations'" },
		{ "Cobra", "Cobra/Cobra_Animations.Cobra_Animations'" },
		{ "Peacock", "Peacock/Peacock_Animations.Peacock_Animations'" },
		{ "Panda", "Panda/Panda_Animations.Panda_Animations'" }
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

	TMap<FString, TMap<FString, UAnimSequence*>*> AnimalAnimationMap = {
			{"Tiger", &TigerLoadedAnimations},
			{"Tapir", &TapirLoadedAnimations},
			{"Gorilla", &GorillaLoadedAnimations},
			{"Bat", &BatLoadedAnimations},
			{"Sloth", &SlothLoadedAnimations},
			{"Cobra", &CobraLoadedAnimations},
			{"Peacock", &PeacockLoadedAnimations},
			{"Panda", &PandaLoadedAnimations}
	};
};



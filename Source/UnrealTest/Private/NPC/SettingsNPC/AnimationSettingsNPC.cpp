#include "AnimationSettingsNPC.h"

UAnimationSettingsNPC::UAnimationSettingsNPC() {
    UE_LOG(LogTemp, Warning, TEXT("Spawning the Animation Settings NPC"));
	LoadAnimationsForAllAnimals();
}

// Destroy all the loaded animation objects and empty the maps for all the NPCs
UAnimationSettingsNPC::~UAnimationSettingsNPC() {
    UE_LOG(LogTemp, Warning, TEXT("Destroying the Animation Settings NPC"));
    for (const FString& animal : Animals) {
        TMap<FString, UAnimSequence*>* animalList = GetAnimalAnimationList(animal);
        if (animalList) {
            animalList->Empty();
        }
    }
}

UAnimSequence* UAnimationSettingsNPC::GetAnimation(const FString& animal, const FString& animationType) {
    TMap<FString, UAnimSequence*>* animalList = GetAnimalAnimationList(animal);
    return (*animalList)[animationType];
}

FString UAnimationSettingsNPC::GetSkeletalMeshPath(const FString& animal) {
    return BaseSkeletalMeshPath + SkeletalPath[animal];
}

void UAnimationSettingsNPC::LoadAnimationsForAllAnimals() {
    for (const FString& animal : Animals) {
        for (int32 i = 0; i < AnimationKeys.Num(); ++i) {

            FString animationPath = BaseAnimationPath + animal + AnimationNamesPath[i];
            UAnimSequence* animSequence = LoadObject<UAnimSequence>(nullptr, *animationPath);
            if (animSequence) {
                TMap<FString, UAnimSequence*>* animalList = GetAnimalAnimationList(animal);
                animalList->Add(AnimationKeys[i], animSequence);
            }
        }
    }
}
TMap<FString, UAnimSequence*>* UAnimationSettingsNPC::GetAnimalAnimationList(const FString& animal) {
    if (AnimalAnimationMap.Contains(animal)) {
        return AnimalAnimationMap[animal];
    }

    return nullptr;
}

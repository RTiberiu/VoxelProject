#include "AnimationSettingsNPC.h"

UAnimationSettingsNPC::UAnimationSettingsNPC() {
    UE_LOG(LogTemp, Warning, TEXT("Spawning the Animation Settings NPC"));
	LoadAnimationsForAllAnimals();
}

// Destroy all the loaded animation objects and empty the maps for all the NPCs
UAnimationSettingsNPC::~UAnimationSettingsNPC() {
    UE_LOG(LogTemp, Warning, TEXT("Destroying the Animation Settings NPC"));
    for (const AnimalType& animal : Animals) {
        TMap<FString, UAnimSequence*>* animalList = GetAnimalAnimationList(animal);
        if (animalList) {
            animalList->Empty();
        }
    }
}

UAnimSequence* UAnimationSettingsNPC::GetAnimation(const AnimalType& animal, const FString& animationType) {
    TMap<FString, UAnimSequence*>* animalList = GetAnimalAnimationList(animal);
    return (*animalList)[animationType];
}

FString UAnimationSettingsNPC::GetSkeletalMeshPath(const AnimalType& animal) {
    return BaseSkeletalMeshPath + SkeletalPath[animal];
}

void UAnimationSettingsNPC::LoadAnimationsForAllAnimals() {
    for (const AnimalType& animal : Animals) {
        for (int32 i = 0; i < AnimationKeys.Num(); i++) {

            FString animationPath = BaseAnimationPath + TypeToName[animal] + AnimationNamesPath[i];
            UAnimSequence* animSequence = LoadObject<UAnimSequence>(nullptr, *animationPath);
            if (animSequence) {
                TMap<FString, UAnimSequence*>* animalList = GetAnimalAnimationList(animal);
                animalList->Add(AnimationKeys[i], animSequence);
            }
        }
    }
}
TMap<FString, UAnimSequence*>* UAnimationSettingsNPC::GetAnimalAnimationList(const AnimalType& animal) {
    if (AnimalAnimationMap.Contains(animal)) {
        return AnimalAnimationMap[animal];
    }

    return nullptr;
}

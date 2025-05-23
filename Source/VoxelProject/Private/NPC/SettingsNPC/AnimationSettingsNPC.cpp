#include "AnimationSettingsNPC.h"

UAnimationSettingsNPC::UAnimationSettingsNPC() {
    UE_LOG(LogTemp, Warning, TEXT("Spawning the Animation Settings NPC"));
	LoadAnimationsForAllAnimals();
}

// Destroy all the loaded animation objects and empty the maps for all the NPCs
UAnimationSettingsNPC::~UAnimationSettingsNPC() {
    UE_LOG(LogTemp, Warning, TEXT("Destroying the Animation Settings NPC"));
    for (const AnimalType& animal : Animals) {
        TMap<AnimationType, UAnimSequence*>* animalList = GetAnimalAnimationList(animal);
        if (animalList) {
            animalList->Empty();
        }
    }
}

UAnimSequence* UAnimationSettingsNPC::GetAnimation(const AnimalType& animal, const AnimationType& animationType) {
    TMap<AnimationType, UAnimSequence*>* animalList = GetAnimalAnimationList(animal);

    // Check if animalList is valid
    if (!animalList) {
        UE_LOG(LogTemp, Warning, TEXT("GetAnimation: Animal list is null for animal type %d"), static_cast<int32>(animal));
        return nullptr;
    }

    // Check if animationType exists in the map
    if (!animalList->Contains(animationType)) {
        UE_LOG(LogTemp, Warning, TEXT("GetAnimation: Animation type %d not found for animal type %d"),
            static_cast<int32>(animationType), static_cast<int32>(animal));
        return nullptr;
    }

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
                TMap<AnimationType, UAnimSequence*>* animalList = GetAnimalAnimationList(animal);
                animalList->Add(AnimationKeys[i], animSequence);
            } else {
                UE_LOG(LogTemp, Error, TEXT("Animation sequence not found: %s"), *animationPath);
            }
        }
    }
}
TMap<AnimationType, UAnimSequence*>* UAnimationSettingsNPC::GetAnimalAnimationList(const AnimalType& animal) {
    if (AnimalAnimationMap.Contains(animal)) {
        return AnimalAnimationMap[animal];
    }

    return nullptr;
}

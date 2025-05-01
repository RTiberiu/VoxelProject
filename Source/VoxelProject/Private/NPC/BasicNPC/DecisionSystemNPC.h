#pragma once

#include "CoreMinimal.h"
#include "..\SettingsNPC\BasePropertiesNPC.h"
#include "..\SettingsNPC\RelationshipSettingsNPC.h"
#include "..\SettingsNPC\AnimationSettingsNPC.h"
#include "..\SettingsNPC\ActionStructures.h"
#include "DecisionSystemNPC.generated.h"

class ABasicNPC;

UCLASS()
class UDecisionSystemNPC : public UObject {
	GENERATED_BODY()

public:
	UDecisionSystemNPC();
	~UDecisionSystemNPC();

	void Initialize(ABasicNPC* InOwner, const AnimalType& animalType);

	NpcAction GetAction(bool ChooseOptimalAction, const int& IncrementTargetInVisionList);

	BasicNpcAttributes AnimalAttributes;
	MemoryNpcAttributes MemoryAttributes;

private:
	ABasicNPC* Owner; 

	// Actions
	FORCENOINLINE NpcAction ShouldFlee(const float& RandomNo);
	FORCENOINLINE NpcAction ShouldRestAfterMeals();
	FORCENOINLINE NpcAction ShouldAttackNpc(const float& RandomNo, bool ChooseOptimalAction, const int& IncrementTargetInVisionList);
	FORCENOINLINE NpcAction ShouldEatBasicFoodSource(const float& RandomNo, bool ChooseOptimalAction, const int& IncrementTargetInVisionList);
	FORCENOINLINE NpcAction ShouldAttemptFoodTrade(const float& RandomNo, bool ChooseOptimalAction, const int& IncrementTargetInVisionList);
	FORCENOINLINE NpcAction ShouldRoam();
	FORCENOINLINE NpcAction ShouldRelax();

	// Used by ShouldRoam and ShouldFlee to select a random location to move
	FVector GetRandomLocationAround(const FVector& Origin, float Radius) const;

	NpcAction FirstValidAction(std::initializer_list<NpcAction> Actions);
	NpcAction NoneAction = NpcAction(FVector::ZeroVector, AnimationType::IdleA, ActionType::NoAction, nullptr);

	// Base and memory attributes for all the animal types
	const TMap<AnimalType, const BasicNpcAttributes*> AnimalsBaseAttributes = {
		{AnimalType::RedPanda, &RedPandaBasicAttributes},
		{AnimalType::Tiger, &TigerBasicAttributes},
		{AnimalType::Tapir, &TapirBasicAttributes},
		{AnimalType::Sloth, &SlothBasicAttributes},
		{AnimalType::Cobra, &CobraBasicAttributes},
		{AnimalType::Bat, &BatBasicAttributes},
		{AnimalType::Peacock, &PeacockBasicAttributes},
		{AnimalType::Gorilla, &GorillaBasicAttributes},
		{AnimalType::Panda, &PandaBasicAttributes}
	};

	const TMap<AnimalType, const MemoryNpcAttributes*> AnimalsMemoryAttributes = {
		{AnimalType::RedPanda, &RedPandaMemoryAttributes},
		{AnimalType::Tiger, &TigerMemoryAttributes},
		{AnimalType::Tapir, &TapirMemoryAttributes},
		{ AnimalType::Sloth,&SlothMemoryAttributes },
		{AnimalType::Cobra, &CobraMemoryAttributes},
		{AnimalType::Bat, &BatMemoryAttributes},
		{AnimalType::Peacock, &PeacockMemoryAttributes},
		{AnimalType::Gorilla, &GorillaMemoryAttributes},
		{AnimalType::Panda, &PandaMemoryAttributes}
	};

	const TMap<FString, const AnimalType*> AnimalsRelationShips = {
		{"TigerFood", &TigerFoodType},
		{"TigerAllies", &TigerAllies},
		{"TigerEnemies", &TigerEnemies},
		{"TapirFood", &TapirFoodType},
		{"TapirAllies", &TapirAllies},
		{"TapirEnemies", &TapirEnemies}

		// TODO Add the rest of the relationships
		// TODO Improve this Map, instead of using FString,
		// use AnimalType and RelationshipType
	};

protected:


};
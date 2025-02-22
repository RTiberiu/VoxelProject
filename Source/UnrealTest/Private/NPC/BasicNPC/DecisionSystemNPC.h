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

	NpcAction GetAction();

	BasicNpcAttributes AnimalAttributes;
	MemoryNpcAttributes MemoryAttributes;

private:
	ABasicNPC* Owner; 

	// How often the NPC should check if their current action should be interrupted and replaced with another action
	FTimerHandle DecisionTimer;
	void ShouldActionBeInterrupted();

	void NotifyNpcOfNewAction();

	// Base and memory attributes for all the animal types
	const TMap<AnimalType, const BasicNpcAttributes*> AnimalsBaseAttributes = {
		{AnimalType::Tiger, &TigerBasicAttributes},
		{AnimalType::Tapir, &TapirBasicAttributes}
	};

	const TMap<AnimalType, const MemoryNpcAttributes*> AnimalsMemoryAttributes = {
		{AnimalType::Tiger, &TigerMemoryAttributes},
		{AnimalType::Tapir, &TapirMemoryAttributes}
	};

	const TMap<FString, const AnimalType*> AnimalsRelationShips = {
		{"TigerFood", &TigerFoodType},
		{"TigerAllies", &TigerAllies},
		{"TigerEnemies", &TigerEnemies},
		{"TapirFood", &TapirFoodType},
		{"TapirAllies", &TapirAllies},
		{"TapirEnemies", &TapirEnemies}
	};

	// Used for roam random directions
	const TArray<FVector> Directions = {
		FVector(1, 0, 0),   // +X
		FVector(-1, 0, 0),  // -X
		FVector(0, 1, 0),   // +Y
		FVector(0, -1, 0),  // -Y
		FVector(1, 1, 0),   // +X, +Y
		FVector(-1, 1, 0),  // -X, +Y
		FVector(1, -1, 0),  // +X, -Y
		FVector(-1, -1, 0)  // -X, -Y
	};

protected:


};
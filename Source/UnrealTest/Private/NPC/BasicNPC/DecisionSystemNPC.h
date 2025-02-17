#pragma once

#include "CoreMinimal.h"
#include "..\SettingsNPC\BasePropertiesNPC.h"
#include "..\SettingsNPC\RelationshipSettingsNPC.h"
#include "DecisionSystemNPC.generated.h"

UCLASS()
class UDecisionSystemNPC : public UObject {
	GENERATED_BODY()

public:
	UDecisionSystemNPC();
	~UDecisionSystemNPC();

	void Initialize(const AnimalType& animalType);

	BasicNpcAttributes AnimalAttributes;
	MemoryNpcAttributes MemoryAttributes;

private:
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

protected:


};
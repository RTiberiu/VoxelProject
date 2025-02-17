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

	void Initialize(const FString& animalType);

	BasicNpcAttributes AnimalAttributes;
	MemoryNpcAttributes MemoryAttributes;

private:
	// Base and memory attributes for all the animal types
	const TMap<FString, const BasicNpcAttributes*> AnimalsBaseAttributes = {
		{"Tiger", &TigerBasicAttributes},
		{"Tapir", &TapirBasicAttributes}
	};

	const TMap<FString, const MemoryNpcAttributes*> AnimalsMemoryAttributes = {
		{"Tiger", &TigerMemoryAttributes},
		{"Tapir", &TapirMemoryAttributes}
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
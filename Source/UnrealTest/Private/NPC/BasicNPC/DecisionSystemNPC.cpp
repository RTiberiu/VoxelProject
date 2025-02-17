#include "DecisionSystemNPC.h"

UDecisionSystemNPC::UDecisionSystemNPC() {
	
}

UDecisionSystemNPC::~UDecisionSystemNPC() {
}

// TODO ADD NOISE TO INFLUENCE THE BASE AND MEMORY ATTRIBUTES
void UDecisionSystemNPC::Initialize(const FString& animalType) {
	AnimalAttributes = *AnimalsBaseAttributes[animalType];
	MemoryAttributes = *AnimalsMemoryAttributes[animalType];
}


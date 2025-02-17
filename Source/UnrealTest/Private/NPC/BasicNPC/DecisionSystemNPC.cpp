#include "DecisionSystemNPC.h"

UDecisionSystemNPC::UDecisionSystemNPC() {
	
}

UDecisionSystemNPC::~UDecisionSystemNPC() {
}

// TODO ADD NOISE TO INFLUENCE THE BASE AND MEMORY ATTRIBUTES
void UDecisionSystemNPC::Initialize(const AnimalType& animalType) {
	AnimalAttributes = *AnimalsBaseAttributes[animalType];
	MemoryAttributes = *AnimalsMemoryAttributes[animalType];
}


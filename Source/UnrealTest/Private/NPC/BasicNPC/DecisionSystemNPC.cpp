#include "DecisionSystemNPC.h"

UDecisionSystemNPC::UDecisionSystemNPC() {
}

UDecisionSystemNPC::~UDecisionSystemNPC() {
}

// TODO ADD NOISE TO INFLUENCE THE BASE AND MEMORY ATTRIBUTES
void UDecisionSystemNPC::Initialize(const ABasicNPC* InOwner, const AnimalType& animalType) {
	Owner = InOwner;
	AnimalAttributes = *AnimalsBaseAttributes[animalType];
	MemoryAttributes = *AnimalsMemoryAttributes[animalType];

	// Set how often the Decision System should check if the action should be interrupted and replaced
	UWorld* World = Owner->GetWorld();
	if (World) {
		World->GetTimerManager().SetTimer(DecisionTimer, this, &UDecisionSystemNPC::ShouldActionBeInterrupted, AnimalAttributes.reactionSpeed, true);
	}
}

void UDecisionSystemNPC::GetAction() {
	// Action order :
	// 
	// 1. Enemy around or chasing->Run
	// 2. Hungry->Get food
	// 3. Tired->Rest
	// 4. Social->Recruit or hoard food

	// TODO Make decision based on current attributes and the vision lists
	// Pass decision to the NPC
	 
	
	// TODO Find an efficient way to check multiple attributes when making a decision
	// Check for hunger 
	if (AnimalAttributes.totalHunger > 50) {
		// Roam
	} else {
		// Find food 

	}

}

void UDecisionSystemNPC::ShouldActionBeInterrupted() {
	// TODO Check if current action should be interrupted 

	// If no, return

	// If yes, remove the current action (clean path and notify to stop) and provide a new action instead.
}

void UDecisionSystemNPC::NotifyNpcOfNewAction() {
	// TODO Notify the owner of the action they need to perform
	//		Something like a location for them to go, and what animation should they run when they're there. 
}

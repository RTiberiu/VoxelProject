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
	 
	// TODO Inside the NPC, they should handle their own attributes
	// like depleting the stamina. In this example, they should 
	// stop and rest when it reaches zero, and then continue with their
	// action. 
	// Similar attribute checks might be needed.

	const float RandomNo = FMath::Rand();

	// Check if NPC should flee from enemies if they exist
	if (Owner->IsThreatInRange()) {
		const bool runFromEnemy = RandomNo < AnimalAttributes.survivalInstinct;
	
		if (runFromEnemy) {
			// TODO Find point to flee 
			return;
		}
	}

	// TODO Check if NPC should rest after food meals
	const bool shouldRestAfterBasicMeal = AnimalAttributes.basicMealsCounter >= AnimalAttributes.restAfterFoodBasic;
	if (shouldRestAfterBasicMeal) {
		// TODO Rest
		AnimalAttributes.basicMealsCounter = 0;
		return;
	}

	const bool shouldRestAfterImprovedMeal = AnimalAttributes.improvedMealsCounter >= AnimalAttributes.restAfterFoodImproved;
	if (shouldRestAfterImprovedMeal) {
		// TODO Rest
		AnimalAttributes.improvedMealsCounter = 0;
		return;
	}

	// TODO Check if NPC should chase for food 
	if (Owner->IsFoodNpcInRange()) {
		const bool shouldChasePrey = RandomNo < AnimalAttributes.chaseDesire;

		if (shouldChasePrey) {
			// TODO Chase prey
			return;
		}
	}

	// Check if NPC should gather food 
	const bool isHungry = AnimalAttributes.totalHunger > 50;
	const bool wantsToHoard = RandomNo < AnimalAttributes.desireToHoardFood;
	
	if (isHungry || wantsToHoard) {
		// TODO Find food 
		return; 
	}

	// Check if NPC should share food for allies
	if (Owner->IsAllyInRange()) {
		const bool shouldGiveFood = RandomNo < AnimalAttributes.desireToRecruitAllies;
		const bool hasEnoughFoodToShare = AnimalAttributes.foodPouch >= AnimalAttributes.foodOfferAmount;

		if (shouldGiveFood && hasEnoughFoodToShare) {
			// TODO Share food with ally in range
			return;
		}
	}

	// TODO The NPC should roam 
	return;
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

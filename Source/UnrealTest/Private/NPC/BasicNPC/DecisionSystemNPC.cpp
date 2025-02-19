#include "DecisionSystemNPC.h"

#include "BasicNPC.h"

UDecisionSystemNPC::UDecisionSystemNPC() {
}

UDecisionSystemNPC::~UDecisionSystemNPC() {
}

// TODO ADD NOISE TO INFLUENCE THE BASE AND MEMORY ATTRIBUTES
void UDecisionSystemNPC::Initialize(ABasicNPC* InOwner, const AnimalType& animalType) {
	Owner = InOwner;
	AnimalAttributes = *AnimalsBaseAttributes[animalType];
	MemoryAttributes = *AnimalsMemoryAttributes[animalType];

	// Set how often the Decision System should check if the action should be interrupted and replaced
	UWorld* World = Owner->GetWorld();
	if (World) {
		World->GetTimerManager().SetTimer(DecisionTimer, this, &UDecisionSystemNPC::ShouldActionBeInterrupted, AnimalAttributes.reactionSpeed, true);
	}
}

NpcAction UDecisionSystemNPC::GetAction() {
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

	// TODO Should check if the locations given (for roam and others) are valid first. This is a maybe.

	const float RandomNo = FMath::Rand();

	// Check if NPC should flee from enemies if they exist
	if (Owner->IsThreatInRange()) {
		const bool runFromEnemy = RandomNo < AnimalAttributes.survivalInstinct;
	
		if (runFromEnemy) {
			// TODO Get the opposite direction of the enemy to not run in the enemy's direction
			
			FVector& CurrentLoc = Owner->GetCurrentLocation();
			FVector fleeingLocation = FVector(CurrentLoc.X + AnimalAttributes.fleeingRadius, CurrentLoc.Y + AnimalAttributes.fleeingRadius, CurrentLoc.Z);
			return NpcAction(fleeingLocation, AnimationType::IdleA);
		}
	}

	// TODO Check if NPC should rest after food meals
	const bool shouldRestAfterBasicMeal = AnimalAttributes.basicMealsCounter >= AnimalAttributes.restAfterFoodBasic;
	if (shouldRestAfterBasicMeal) {
		
		AnimalAttributes.basicMealsCounter = 0;
		return NpcAction(Owner->GetCurrentLocation(), AnimationType::Sit);
	}

	const bool shouldRestAfterImprovedMeal = AnimalAttributes.improvedMealsCounter >= AnimalAttributes.restAfterFoodImproved;
	if (shouldRestAfterImprovedMeal) {
		// TODO Rest
		AnimalAttributes.improvedMealsCounter = 0;
		return NpcAction(Owner->GetCurrentLocation(), AnimationType::Sit);
	}

	// TODO Check if NPC should chase for food 
	if (Owner->IsFoodNpcInRange()) {
		const bool shouldChasePrey = RandomNo < AnimalAttributes.chaseDesire;

		if (shouldChasePrey) {
			ABasicNPC* TargetPrey = std::get<ABasicNPC*>(Owner->GetClosestInVisionList(VisionList::NpcFood));
			return NpcAction(TargetPrey->GetCurrentLocation(), AnimationType::Attack);
		}
	}

	// Check if NPC should gather food 
	const bool isHungry = AnimalAttributes.totalHunger > 50;
	const bool wantsToHoard = RandomNo < AnimalAttributes.desireToHoardFood;
	
	if (isHungry || wantsToHoard) {
		// TODO Find food 
		UPrimitiveComponent* TargetFood = std::get<UPrimitiveComponent*>(Owner->GetClosestInVisionList(VisionList::FoodSource));
		return NpcAction(TargetFood->GetComponentLocation(), AnimationType::Attack);
	}

	// Check if NPC should share food for allies
	if (Owner->IsAllyInRange()) {
		const bool shouldGiveFood = RandomNo < AnimalAttributes.desireToRecruitAllies;
		const bool hasEnoughFoodToShare = AnimalAttributes.foodPouch >= AnimalAttributes.foodOfferAmount;

		if (shouldGiveFood && hasEnoughFoodToShare) {
			// TODO Share food with ally in range
			ABasicNPC* TargetAlly = std::get<ABasicNPC*>(Owner->GetClosestInVisionList(VisionList::Allies));
			return NpcAction(TargetAlly->GetCurrentLocation(), AnimationType::Spin);
		}
	}

	// TODO The NPC should 
	FVector& CurrentLoc = Owner->GetCurrentLocation();
	FVector RoamLocation = FVector(CurrentLoc.X + AnimalAttributes.roamRadius, CurrentLoc.Y + AnimalAttributes.roamRadius, CurrentLoc.Z);
	return NpcAction(RoamLocation, AnimationType::IdleA);
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

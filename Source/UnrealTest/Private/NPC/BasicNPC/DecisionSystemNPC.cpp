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

NpcAction UDecisionSystemNPC::GetAction(bool ChooseOptimalAction, const int& IncrementTargetInVisionList) {
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

	const float RandomNo = FMath::FRand();

	return FirstValidAction({
		ShouldFlee(RandomNo), // Check if NPC should flee from enemies if they exist
		//ShouldRestAfterMeals(), // Check if NPC should rest after basic or improved food meals
		ShouldAttackNpc(RandomNo, ChooseOptimalAction, IncrementTargetInVisionList), // Check if NPC should chase for food 
		//ShouldEatBasicFoodSource(RandomNo), // Check if NPC should gather food 
		//ShouldAttemptFoodTrade(RandomNo), // Check if NPC should share food for allies
		//ShouldRoam(), // The NPC should roam in a random direction
		ShouldRelax() // For testing only
		});
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

NpcAction UDecisionSystemNPC::ShouldFlee(const float& RandomNo) {
	//UE_LOG(LogTemp, Warning, TEXT("ShouldFlee function is executed."));

	// Check if NPC should flee from enemies if they exist
	if (Owner->IsThreatInRange()) {
		//UE_LOG(LogTemp, Warning, TEXT("\tThreat is in range!."));

		//UE_LOG(LogTemp, Warning, TEXT("\t\tRandomNo: %f"), RandomNo);
		//UE_LOG(LogTemp, Warning, TEXT("\t\tAnimalAttributes.survivalInstinct: %f"), AnimalAttributes.survivalInstinct);

		const bool runFromEnemy = RandomNo < AnimalAttributes.survivalInstinct;
		if (runFromEnemy) {
			//UE_LOG(LogTemp, Warning, TEXT("\t\tShould run from enemy!."));

			// TODO Get the opposite direction of the enemy to not run in the enemy's direction
			FVector& CurrentLoc = Owner->GetCurrentLocation();
			FVector fleeingLocation = FVector(CurrentLoc.X + AnimalAttributes.fleeingRadius, CurrentLoc.Y + AnimalAttributes.fleeingRadius, CurrentLoc.Z);
			return NpcAction(fleeingLocation, AnimationType::IdleA, ActionType::Flee, nullptr);
		}
	}

	return NoneAction;
}

NpcAction UDecisionSystemNPC::ShouldRestAfterMeals() {
	const bool shouldRestAfterBasicMeal = AnimalAttributes.basicMealsCounter >= AnimalAttributes.restAfterFoodBasic;
	if (shouldRestAfterBasicMeal) {
		AnimalAttributes.basicMealsCounter = 0;
		return NpcAction(Owner->GetCurrentLocation(), AnimationType::Sit, ActionType::RestAfterBasicFood, nullptr);
	}

	const bool shouldRestAfterImprovedMeal = AnimalAttributes.improvedMealsCounter >= AnimalAttributes.restAfterFoodImproved;
	if (shouldRestAfterImprovedMeal) {
		AnimalAttributes.improvedMealsCounter = 0;
		return NpcAction(Owner->GetCurrentLocation(), AnimationType::Sit, ActionType::RestAfterImprovedFood, nullptr);
	}

	return NoneAction;
}

NpcAction UDecisionSystemNPC::ShouldAttackNpc(const float& RandomNo, bool ChooseOptimalAction, const int& IncrementTargetInVisionList) {
	//UE_LOG(LogTemp, Warning, TEXT("ShouldAttackNpc function is executed."));

	// Check if NPC should chase for food 
	if (Owner->IsFoodNpcInRange()) {
		const bool shouldChasePrey = RandomNo < AnimalAttributes.chaseDesire;

		if (shouldChasePrey) {
			std::variant<ABasicNPC*, UCustomProceduralMeshComponent*> ClosestVariant = Owner->GetClosestInVisionList(VisionList::NpcFood, ChooseOptimalAction, IncrementTargetInVisionList);
			ABasicNPC* TargetPrey = std::get<ABasicNPC*>(ClosestVariant);

			if (TargetPrey == nullptr) return NoneAction;
			return NpcAction(TargetPrey->GetCurrentLocation(), AnimationType::Attack, ActionType::AttackNpc, TargetPrey);
		}
	}

	return NoneAction;
}

NpcAction UDecisionSystemNPC::ShouldEatBasicFoodSource(const float& RandomNo, bool ChooseOptimalAction, const int& IncrementTargetInVisionList) {
	// Check if NPC should gather food 
	if (Owner->IsFoodSourceInRange()) {
		const bool isHungry = AnimalAttributes.currentHunger > 50;
		const bool wantsToHoard = RandomNo < AnimalAttributes.desireToHoardFood;

		if (isHungry || wantsToHoard) {
			// Find food 
			std::variant<ABasicNPC*, UCustomProceduralMeshComponent*> ClosestVariant = Owner->GetClosestInVisionList(VisionList::FoodSource, ChooseOptimalAction, IncrementTargetInVisionList);

			UCustomProceduralMeshComponent* TargetFood = std::get<UCustomProceduralMeshComponent*>(ClosestVariant);
			return NpcAction(TargetFood->GetComponentLocation(), AnimationType::Attack, ActionType::AttackFoodSource, TargetFood);
		}
	}

	return NoneAction;
}

NpcAction UDecisionSystemNPC::ShouldAttemptFoodTrade(const float& RandomNo, bool ChooseOptimalAction, const int& IncrementTargetInVisionList) {
   // Check if NPC should share food for allies  
   if (Owner->IsAllyInRange()) {  
       const bool shouldGiveFood = RandomNo < AnimalAttributes.desireToRecruitAllies;  
       const bool hasEnoughFoodToShare = AnimalAttributes.foodPouch >= AnimalAttributes.foodOfferAmount;  

       if (shouldGiveFood && hasEnoughFoodToShare) {  
           // Share food with ally in range  
           std::variant<ABasicNPC*, UCustomProceduralMeshComponent*> ClosestVariant = Owner->GetClosestInVisionList(VisionList::Allies, ChooseOptimalAction, IncrementTargetInVisionList);
           ABasicNPC* TargetAlly = std::get<ABasicNPC*>(ClosestVariant);  
           return NpcAction(TargetAlly->GetCurrentLocation(), AnimationType::Spin, ActionType::TradeFood, TargetAlly);  
       }  
   }  

   return NoneAction;
}

NpcAction UDecisionSystemNPC::ShouldRoam() {
	// TODO Improve this so it doesn't always go in a straight line. 
	// Potentially reduce the roamRadius on each axis by the RandomNo

	// The NPC should roam in a random direction
	FVector& CurrentLoc = Owner->GetCurrentLocation();
	const int RandomDirection = FMath::RandRange(0, 7);
	FVector RoamLocation = CurrentLoc + (Directions[RandomDirection] * AnimalAttributes.roamRadius);

	return NpcAction(RoamLocation, AnimationType::Walk, ActionType::Roam, nullptr);
}

NpcAction UDecisionSystemNPC::ShouldRelax() {
	//UE_LOG(LogTemp, Warning, TEXT("ShouldRelax function is executed."));

	FVector& CurrentLoc = Owner->GetCurrentLocation();
	return NpcAction(CurrentLoc, AnimationType::Sit, ActionType::RestAfterBasicFood, nullptr);
}

NpcAction UDecisionSystemNPC::FirstValidAction(std::initializer_list<NpcAction> Actions) {
	for (const NpcAction& Action : Actions) {
		if (Action.ActionType != ActionType::NoAction) {
			return Action;
		}
	}
	return NoneAction;
}

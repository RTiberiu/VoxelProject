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
	const float RandomNo = FMath::FRand();

	return FirstValidAction({
		ShouldFlee(RandomNo), // Check if NPC should flee from enemies if they exist
		//ShouldRestAfterMeals(), // Check if NPC should rest after basic or improved food meals
		ShouldAttackNpc(RandomNo, ChooseOptimalAction, IncrementTargetInVisionList), // Check if NPC should chase for food 
		//ShouldEatBasicFoodSource(RandomNo, ChooseOptimalAction, IncrementTargetInVisionList), // Check if NPC should gather food 
		//ShouldAttemptFoodTrade(RandomNo, ChooseOptimalAction, IncrementTargetInVisionList), // Check if NPC should share food for allies
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
	// Check if NPC should flee from enemies if they exist
	if (!Owner->IsThreatInRange()) {
		return NoneAction;
	}

	const bool runFromEnemy = RandomNo < AnimalAttributes.survivalInstinct;
	if (!runFromEnemy) {
		return NoneAction;
	}

	const bool shouldNotifyOtherNpcs = RandomNo < AnimalAttributes.communicationDesire;

	// Flee in a random direction
	FVector CurrentLoc = Owner->GetCurrentLocation();
	FVector FleeLocRandom = GetRandomLocationAround(CurrentLoc, AnimalAttributes.fleeingRadius);
    return NpcAction(FleeLocRandom, AnimationType::IdleA, ActionType::Flee, nullptr, shouldNotifyOtherNpcs);
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
	if (!Owner->IsFoodNpcInRange()) {
		return NoneAction;
	}

	const bool shouldChasePrey = RandomNo < AnimalAttributes.chaseDesire;
	if (!shouldChasePrey) {
		return NoneAction;
	}

	std::variant<ABasicNPC*, UCustomProceduralMeshComponent*> ClosestVariant =
		Owner->GetClosestInVisionList(VisionList::NpcFood, ChooseOptimalAction, IncrementTargetInVisionList);

	// Only proceed if the variant holds an ABasicNPC*
	if (!std::holds_alternative<ABasicNPC*>(ClosestVariant)) {
		return NoneAction;
	}

	ABasicNPC* TargetPrey = std::get<ABasicNPC*>(ClosestVariant);
	if (TargetPrey == nullptr) {
		return NoneAction;
	}

	const bool shouldNotifyOtherNpcs = RandomNo < AnimalAttributes.communicationDesire;

	return NpcAction(
		TargetPrey->GetCurrentLocation(),
		AnimationType::Attack,
		ActionType::AttackNpc,
		TargetPrey,
		shouldNotifyOtherNpcs
	);
}


NpcAction UDecisionSystemNPC::ShouldEatBasicFoodSource(const float& RandomNo, bool ChooseOptimalAction, const int& IncrementTargetInVisionList) {
	// Check if NPC should gather food 
	if (!Owner->IsFoodSourceInRange()) {
		return NoneAction;
	}

	const bool isHungry = AnimalAttributes.currentHunger > 50;
	const bool wantsToHoard = RandomNo < AnimalAttributes.desireToHoardFood;

	if (!(isHungry || wantsToHoard)) {
		return NoneAction;
	}

	// Find food
	std::variant<ABasicNPC*, UCustomProceduralMeshComponent*>  ClosestVariant =
		Owner->GetClosestInVisionList(
			VisionList::FoodSource,
			ChooseOptimalAction,
			IncrementTargetInVisionList);

	if (std::holds_alternative<UCustomProceduralMeshComponent*>(ClosestVariant)) {
		UCustomProceduralMeshComponent* TargetFood =
			std::get<UCustomProceduralMeshComponent*>(ClosestVariant);

		if (TargetFood) {

			const bool shouldNotifyOtherNpcs = RandomNo < AnimalAttributes.communicationDesire;

			return NpcAction(
				TargetFood->GetComponentLocation(),
				AnimationType::Attack,
				ActionType::AttackFoodSource,
				TargetFood,
				shouldNotifyOtherNpcs
			);
		}
	}

	return NoneAction;
}

NpcAction UDecisionSystemNPC::ShouldAttemptFoodTrade(const float& RandomNo, bool ChooseOptimalAction, const int& IncrementTargetInVisionList) {
	// Check if NPC should share food for allies  
	if (!Owner->IsAllyInRange()) {
		return NoneAction;
	}

	const bool shouldGiveFood = RandomNo < AnimalAttributes.desireToRecruitAllies;
	const bool hasEnoughFoodToShare = AnimalAttributes.foodPouch >= AnimalAttributes.foodOfferAmount;
	if (!(shouldGiveFood && hasEnoughFoodToShare)) {
		return NoneAction;
	}

	// Share food with ally in range  
	std::variant<ABasicNPC*, UCustomProceduralMeshComponent*> ClosestVariant =
		Owner->GetClosestInVisionList(VisionList::Allies, ChooseOptimalAction, IncrementTargetInVisionList);

	// Only proceed if the variant holds an ABasicNPC*
	if (!std::holds_alternative<ABasicNPC*>(ClosestVariant)) {
		return NoneAction;
	}

	ABasicNPC* TargetAlly = std::get<ABasicNPC*>(ClosestVariant);
	if (TargetAlly == nullptr) {
		return NoneAction;
	}

	// This action notifies by default, so a check is not needed for communicationDesire
	return NpcAction(
		TargetAlly->GetCurrentLocation(),
		AnimationType::Spin,
		ActionType::TradeFood,
		TargetAlly
	);
}


NpcAction UDecisionSystemNPC::ShouldRoam() {
	// TODO Improve this so it doesn't always go in a straight line. 
	// Potentially reduce the roamRadius on each axis by the RandomNo

	// The NPC should roam in a random direction
	FVector& CurrentLoc = Owner->GetCurrentLocation();
	FVector RoamLocation = GetRandomLocationAround(CurrentLoc, AnimalAttributes.roamRadius);

	return NpcAction(RoamLocation, AnimationType::Walk, ActionType::Roam, nullptr);
}

NpcAction UDecisionSystemNPC::ShouldRelax() {
	//UE_LOG(LogTemp, Warning, TEXT("ShouldRelax function is executed."));

	FVector& CurrentLoc = Owner->GetCurrentLocation();
	return NpcAction(CurrentLoc, AnimationType::Sit, ActionType::RestAfterBasicFood, nullptr);
}

FVector UDecisionSystemNPC::GetRandomLocationAround(const FVector& Origin, float Radius) const {
	// Pick a random angle
	float RandomAngle = FMath::FRandRange(0.0f, 360.0f);

	// Start with +X unit vector and rotate around Z
	FVector Dir2D = FVector(1.0f, 0.0f, 0.0f).RotateAngleAxis(RandomAngle, FVector::UpVector);

	// Scale out and preserve original Z
	FVector Result = Origin + Dir2D * Radius;
	Result.Z = Origin.Z;
	return Result;
}

NpcAction UDecisionSystemNPC::FirstValidAction(std::initializer_list<NpcAction> Actions) {
	for (const NpcAction& Action : Actions) {
		if (Action.ActionType != ActionType::NoAction) {
			return Action;
		}
	}
	return NoneAction;
}

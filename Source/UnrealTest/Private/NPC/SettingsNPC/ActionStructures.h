#pragma once

#include "AnimationSettingsNPC.h"
#include "CoreMinimal.h"

enum ActionType {
	AttackNpc,
	AttackFoodSource,
	Rest,
	TradeFood,
	Roam,
	Flee
};

struct NpcAction {
	FVector TargetLocation;
	AnimationType AnimationToRunAtTarget;
	ActionType ActionType;
	UObject* Target;
};
#pragma once

#include "AnimationSettingsNPC.h"
#include "CoreMinimal.h"

enum ActionType {
	AttackNpc,
	AttackFoodSource,
	RestAfterBasicFood,
	RestAfterImprovedFood,
	TradeFood,
	Roam,
	Flee,
	NoAction
};

struct NpcAction {
	FVector TargetLocation;
	AnimationType AnimationToRunAtTarget;
	ActionType ActionType;
	UObject* Target;
};
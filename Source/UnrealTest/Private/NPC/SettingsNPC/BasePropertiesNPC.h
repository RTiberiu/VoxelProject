#pragma once
#include <cstdint>

struct BasicNpcAttributes {
	uint8_t currentHp;
	uint8_t maxHp;
	uint8_t hitDamage;				// Damage point when attacking another NPC
	uint8_t attackSpeed;			// How long the animal has to wait before dealing another attack

	uint8_t movementSpeed;			// How fast the animal can move in the world.

	uint8_t currentStamina;
	uint8_t maxStamina;
	uint8_t staminaDepletionRate;
	uint8_t staminaRecoveryRate;	
	uint8_t currentHunger;			// The current hunger value
	uint8_t maxHunger;				// The total hunger bar
	uint8_t hungerDepletionRate;
	uint8_t hungerRecoveryBasic;	// How much hunger is restored. Used for basic food sources, like grass or flowers. 
	uint8_t hungerRecoveryImproved;	// How much hunger is restored. Used for improved food sources, like other animals.
	uint8_t eatingSpeedRateBasic;	// How fast the animal will consume the basic food sources
	uint8_t eatingSpeedRateImproved;// How fast the animal will consume the improved food sources

	uint8_t restAfterFoodBasic;		// How long should the NPC rest after eating a basic food source.
	uint8_t restAfterFoodImproved;	// How long should the NPC rest after eating an improved food source.
	uint8_t restAfterStaminaIsZero;	// How long should the NPC rest after the stamina reaches zero.

	uint8_t mealsUntilRestIsNeeded; // After how many meals the NPC should rest.

	uint8_t basicMealsCounter;		// This value resets after resting.
	uint8_t improvedMealsCounter;	// This value resets after resting.

	int foodPouch;					// How much food is stored in the food pouch. This is an overflow that an NPC can store and use as a reserve or for trading.

	float desireToHoardFood;		// (0 - 1) - How much should the NPC want to keep collecting food after the total hunger is satisfied

	float chaseDesire;			// How willing is the animal to chase other targets (animal food sources)
	
	float awarenessRadius;		// The sphere radius that determines which NPCs or objects are being tracked by the NPC
	
	uint16_t fleeingRadius;			// (60 * N) - The radius around the NPC from which to pick a point to run towards when being chased. It has to be a multiply of the UnrealScale chosen

	float reactionSpeed;			// How fast should the NPC react to the environment. This determines how quickly the should intrerupt their current action and replace it with another (for example fleeing from an enemy). In reality, this is the time in seconds the NPC will make decisions

	uint8_t willingnessToBecomeAlly; // How willing is the NPC to become someone else's ally and become a pack

	float desireToRecruitAllies;	// (0-1) - How willing is the animal to trade food from the food pouch to attempt to recruit allies

	float survivalInstinct; // (0-1) - Probability to run when a threat is close.

	uint8_t foodOfferAmount; // (10-50) How much food should it offer to an NPC to convince to become ally.

	uint16_t roamRadius; // (60 * N) - How far away should the NPC roam from its current location. It has to be a multiply of the UnrealScale chosen
};

// Values that change the BasicNpcAttributes based on the outcome of certain actions
struct MemoryNpcAttributes {
	uint8_t negativeChaseDesireBonus; // Rate for decreasing the chase desire when a chase is not successful 

	uint8_t positiveChaseDesireBonus; // Rate for increasing the chase desire when a chase is successful

	uint8_t positiveDesireToRecruitAlliesBonus; // Rate for increasing the desire to recruit allies when a recruitment is successful

	uint8_t negativeDesireToRecruitAlliesBonus; // Rate for decreasing the desire to recruit allies when a recruitment is not successful

	uint8_t positiveDesireToHoardFoodBonus;		// Rate for increasing food hoarding desire if animal gets too close to dying

	// TODO Should find a reason to decrease hoarding desire (maybe animal becomes slower?) 
};

const BasicNpcAttributes TigerBasicAttributes = {
	100,	// currentHp
	100,	// maxHp
	30,		// hitDamage
	5,		// attackSpeed
	200,	// movementSpeed
	100,	// currentStamina
	100, 	// maxStamina
	5,		// staminaDepletionRate
	10,		// staminaRecoveryRate
	100,	// currentHunger
	100,	// maxHunger
	5,		// hungerDepletionRate
	20,		// hungerRecoveryBasic
	30,		// hungerRecoveryImproved
	2,		// eatingSpeedRateBasic
	3,		// eatingSpeedRateImproved
	4,		// restAfterFoodBasic
	6,		// restAfterFoodImproved
	5, 		// restAfterStaminaIsZero
	4,		// mealsUntilRestIsNeeded
	0,		// basicMealsCounter
	0,		// improvedMealsCounter
	0,		// foodPouch
	0.9f,   // desireToHoardFood
	0.9f,	// chaseDesire
	500.0f, // awarenessRadius
	3000,	// fleeingRadius
	0.5f,   // reactionSpeed
	10,		// willingnessToBecomeAlly
	0.1f,	// desireToRecruitAllies
	1.0f,	// survivalInstinct
	30,		// foodOfferAmount
	600,	// roamRadius
};

const MemoryNpcAttributes TigerMemoryAttributes = {
	5,  // negativeChaseDesireBonus
	10, // positiveChaseDesireBonus
	8,  // positiveDesireToRecruitAlliesBonus
	3,  // negativeDesireToRecruitAlliesBonus
	7   // positiveDesireToHoardFoodBonus
};

const BasicNpcAttributes TapirBasicAttributes = {
	100,	// currentHp
	100,	// maxHp
	10,		// hitDamage
	3,		// attackSpeed
	200,	// movementSpeed
	60,		// currentStamina
	60, 	// maxStamina
	5,		// staminaDepletionRate
	10,		// staminaRecoveryRate
	100,	// currentHunger
	100,	// maxHunger
	5,		// hungerDepletionRate
	20,		// hungerRecoveryBasic
	30,		// hungerRecoveryImproved
	2,		// eatingSpeedRateBasic
	3,		// eatingSpeedRateImproved
	3,		// restAfterFoodBasic
	5,		// restAfterFoodImproved
	5, 		// restAfterStaminaIsZero
	4,		// mealsUntilRestIsNeeded
	0,		// basicMealsCounter
	0,		// improvedMealsCounter
	0,		// foodPouch
	0.5f,   // desireToHoardFood
	0.1f,	// chaseDesire
	900.0f, // awarenessRadius
	3000,	// fleeingRadius
	1.0f,   // reactionSpeed
	60,		// willingnessToBecomeAlly
	0.5,	// desireToRecruitAllies
	0.7f,	// survivalInstinct
	20,		// foodOfferAmount
	1500,	// roamRadius
};

const MemoryNpcAttributes TapirMemoryAttributes = {
	3,  // negativeChaseDesireBonus
	6,  // positiveChaseDesireBonus
	5,  // positiveDesireToRecruitAlliesBonus
	2,  // negativeDesireToRecruitAlliesBonus
	4   // positiveDesireToHoardFoodBonus
};

const BasicNpcAttributes SlothBasicAttributes = {
	100,	// currentHp
	100,	// maxHp
	10,		// hitDamage
	3,		// attackSpeed
	200,	// movementSpeed
	60,		// currentStamina
	60, 	// maxStamina
	5,		// staminaDepletionRate
	10,		// staminaRecoveryRate
	100,	// currentHunger
	100,	// maxHunger
	5,		// hungerDepletionRate
	20,		// hungerRecoveryBasic
	30,		// hungerRecoveryImproved
	2,		// eatingSpeedRateBasic
	3,		// eatingSpeedRateImproved
	3,		// restAfterFoodBasic
	5,		// restAfterFoodImproved
	5, 		// restAfterStaminaIsZero
	4,		// mealsUntilRestIsNeeded
	0,		// basicMealsCounter
	0,		// improvedMealsCounter
	0,		// foodPouch
	0.5f,   // desireToHoardFood
	0.1f,	// chaseDesire
	900.0f, // awarenessRadius
	3000,	// fleeingRadius
	1.0f,   // reactionSpeed
	60,		// willingnessToBecomeAlly
	0.5,	// desireToRecruitAllies
	0.7f,	// survivalInstinct
	20,		// foodOfferAmount
	1500,	// roamRadius
};

const MemoryNpcAttributes SlothMemoryAttributes = {
	3,  // negativeChaseDesireBonus
	6,  // positiveChaseDesireBonus
	5,  // positiveDesireToRecruitAlliesBonus
	2,  // negativeDesireToRecruitAlliesBonus
	4   // positiveDesireToHoardFoodBonus
};


const BasicNpcAttributes CobraBasicAttributes = {
	100,	// currentHp
	100,	// maxHp
	10,		// hitDamage
	3,		// attackSpeed
	200,	// movementSpeed
	60,		// currentStamina
	60, 	// maxStamina
	5,		// staminaDepletionRate
	10,		// staminaRecoveryRate
	100,	// currentHunger
	100,	// maxHunger
	5,		// hungerDepletionRate
	20,		// hungerRecoveryBasic
	30,		// hungerRecoveryImproved
	2,		// eatingSpeedRateBasic
	3,		// eatingSpeedRateImproved
	3,		// restAfterFoodBasic
	5,		// restAfterFoodImproved
	5, 		// restAfterStaminaIsZero
	4,		// mealsUntilRestIsNeeded
	0,		// basicMealsCounter
	0,		// improvedMealsCounter
	0,		// foodPouch
	0.5f,   // desireToHoardFood
	0.1f,	// chaseDesire
	900.0f, // awarenessRadius
	3000,	// fleeingRadius
	1.0f,   // reactionSpeed
	60,		// willingnessToBecomeAlly
	0.5,	// desireToRecruitAllies
	0.7f,	// survivalInstinct
	20,		// foodOfferAmount
	1500,	// roamRadius
};

const MemoryNpcAttributes CobraMemoryAttributes = {
	3,  // negativeChaseDesireBonus
	6,  // positiveChaseDesireBonus
	5,  // positiveDesireToRecruitAlliesBonus
	2,  // negativeDesireToRecruitAlliesBonus
	4   // positiveDesireToHoardFoodBonus
};


const BasicNpcAttributes BatBasicAttributes = {
	100,	// currentHp
	100,	// maxHp
	10,		// hitDamage
	3,		// attackSpeed
	200,	// movementSpeed
	60,		// currentStamina
	60, 	// maxStamina
	5,		// staminaDepletionRate
	10,		// staminaRecoveryRate
	100,	// currentHunger
	100,	// maxHunger
	5,		// hungerDepletionRate
	20,		// hungerRecoveryBasic
	30,		// hungerRecoveryImproved
	2,		// eatingSpeedRateBasic
	3,		// eatingSpeedRateImproved
	3,		// restAfterFoodBasic
	5,		// restAfterFoodImproved
	5, 		// restAfterStaminaIsZero
	4,		// mealsUntilRestIsNeeded
	0,		// basicMealsCounter
	0,		// improvedMealsCounter
	0,		// foodPouch
	0.5f,   // desireToHoardFood
	0.1f,	// chaseDesire
	900.0f, // awarenessRadius
	3000,	// fleeingRadius
	1.0f,   // reactionSpeed
	60,		// willingnessToBecomeAlly
	0.5,	// desireToRecruitAllies
	0.7f,	// survivalInstinct
	20,		// foodOfferAmount
	1500,	// roamRadius
};

const MemoryNpcAttributes BatMemoryAttributes = {
	3,  // negativeChaseDesireBonus
	6,  // positiveChaseDesireBonus
	5,  // positiveDesireToRecruitAlliesBonus
	2,  // negativeDesireToRecruitAlliesBonus
	4   // positiveDesireToHoardFoodBonus
};


const BasicNpcAttributes PeacockBasicAttributes = {
	100,	// currentHp
	100,	// maxHp
	10,		// hitDamage
	3,		// attackSpeed
	200,	// movementSpeed
	60,		// currentStamina
	60, 	// maxStamina
	5,		// staminaDepletionRate
	10,		// staminaRecoveryRate
	100,	// currentHunger
	100,	// maxHunger
	5,		// hungerDepletionRate
	20,		// hungerRecoveryBasic
	30,		// hungerRecoveryImproved
	2,		// eatingSpeedRateBasic
	3,		// eatingSpeedRateImproved
	3,		// restAfterFoodBasic
	5,		// restAfterFoodImproved
	5, 		// restAfterStaminaIsZero
	4,		// mealsUntilRestIsNeeded
	0,		// basicMealsCounter
	0,		// improvedMealsCounter
	0,		// foodPouch
	0.5f,   // desireToHoardFood
	0.1f,	// chaseDesire
	900.0f, // awarenessRadius
	3000,	// fleeingRadius
	1.0f,   // reactionSpeed
	60,		// willingnessToBecomeAlly
	0.5,	// desireToRecruitAllies
	0.7f,	// survivalInstinct
	20,		// foodOfferAmount
	1500,	// roamRadius
};

const MemoryNpcAttributes PeacockMemoryAttributes = {
	3,  // negativeChaseDesireBonus
	6,  // positiveChaseDesireBonus
	5,  // positiveDesireToRecruitAlliesBonus
	2,  // negativeDesireToRecruitAlliesBonus
	4   // positiveDesireToHoardFoodBonus
};


const BasicNpcAttributes GorillaBasicAttributes = {
	100,	// currentHp
	100,	// maxHp
	10,		// hitDamage
	3,		// attackSpeed
	200,	// movementSpeed
	60,		// currentStamina
	60, 	// maxStamina
	5,		// staminaDepletionRate
	10,		// staminaRecoveryRate
	100,	// currentHunger
	100,	// maxHunger
	5,		// hungerDepletionRate
	20,		// hungerRecoveryBasic
	30,		// hungerRecoveryImproved
	2,		// eatingSpeedRateBasic
	3,		// eatingSpeedRateImproved
	3,		// restAfterFoodBasic
	5,		// restAfterFoodImproved
	5, 		// restAfterStaminaIsZero
	4,		// mealsUntilRestIsNeeded
	0,		// basicMealsCounter
	0,		// improvedMealsCounter
	0,		// foodPouch
	0.5f,   // desireToHoardFood
	0.1f,	// chaseDesire
	900.0f, // awarenessRadius
	3000,	// fleeingRadius
	1.0f,   // reactionSpeed
	60,		// willingnessToBecomeAlly
	0.5,	// desireToRecruitAllies
	0.7f,	// survivalInstinct
	20,		// foodOfferAmount
	1500,	// roamRadius
};

const MemoryNpcAttributes GorillaMemoryAttributes = {
	3,  // negativeChaseDesireBonus
	6,  // positiveChaseDesireBonus
	5,  // positiveDesireToRecruitAlliesBonus
	2,  // negativeDesireToRecruitAlliesBonus
	4   // positiveDesireToHoardFoodBonus
};


const BasicNpcAttributes PandaBasicAttributes = {
	100,	// currentHp
	100,	// maxHp
	10,		// hitDamage
	3,		// attackSpeed
	200,	// movementSpeed
	60,		// currentStamina
	60, 	// maxStamina
	5,		// staminaDepletionRate
	10,		// staminaRecoveryRate
	100,	// currentHunger
	100,	// maxHunger
	5,		// hungerDepletionRate
	20,		// hungerRecoveryBasic
	30,		// hungerRecoveryImproved
	2,		// eatingSpeedRateBasic
	3,		// eatingSpeedRateImproved
	3,		// restAfterFoodBasic
	5,		// restAfterFoodImproved
	5, 		// restAfterStaminaIsZero
	4,		// mealsUntilRestIsNeeded
	0,		// basicMealsCounter
	0,		// improvedMealsCounter
	0,		// foodPouch
	0.5f,   // desireToHoardFood
	0.1f,	// chaseDesire
	900.0f, // awarenessRadius
	3000,	// fleeingRadius
	1.0f,   // reactionSpeed
	60,		// willingnessToBecomeAlly
	0.5,	// desireToRecruitAllies
	0.7f,	// survivalInstinct
	20,		// foodOfferAmount
	1500,	// roamRadius
};

const MemoryNpcAttributes PandaMemoryAttributes = {
	3,  // negativeChaseDesireBonus
	6,  // positiveChaseDesireBonus
	5,  // positiveDesireToRecruitAlliesBonus
	2,  // negativeDesireToRecruitAlliesBonus
	4   // positiveDesireToHoardFoodBonus
};


const BasicNpcAttributes RedPandaBasicAttributes = {
	100,	// currentHp
	100,	// maxHp
	10,		// hitDamage
	3,		// attackSpeed
	200,	// movementSpeed
	60,		// currentStamina
	60, 	// maxStamina
	5,		// staminaDepletionRate
	10,		// staminaRecoveryRate
	100,	// currentHunger
	100,	// maxHunger
	5,		// hungerDepletionRate
	20,		// hungerRecoveryBasic
	30,		// hungerRecoveryImproved
	2,		// eatingSpeedRateBasic
	3,		// eatingSpeedRateImproved
	3,		// restAfterFoodBasic
	5,		// restAfterFoodImproved
	5, 		// restAfterStaminaIsZero
	4,		// mealsUntilRestIsNeeded
	0,		// basicMealsCounter
	0,		// improvedMealsCounter
	0,		// foodPouch
	0.5f,   // desireToHoardFood
	0.1f,	// chaseDesire
	900.0f, // awarenessRadius
	3000,	// fleeingRadius
	1.0f,   // reactionSpeed
	60,		// willingnessToBecomeAlly
	0.5,	// desireToRecruitAllies
	0.7f,	// survivalInstinct
	20,		// foodOfferAmount
	1500,	// roamRadius
};

const MemoryNpcAttributes RedPandaMemoryAttributes = {
	3,  // negativeChaseDesireBonus
	6,  // positiveChaseDesireBonus
	5,  // positiveDesireToRecruitAlliesBonus
	2,  // negativeDesireToRecruitAlliesBonus
	4   // positiveDesireToHoardFoodBonus
};

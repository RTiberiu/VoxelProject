#pragma once
#include <cstdint>

// TODO ASSIGN BASE PROPERTIES FOR EACH TYPE OF NPC

struct BasicNpcAttributes {
	uint8_t totalHp;
	uint8_t hitDamage;				// Damage point when attacking another NPC
	uint8_t attackSpeed;			// How long the animal has to wait before dealing another attack

	uint8_t movementSpeed;			// How fast the animal can move in the world.

	uint8_t totalStamina;
	uint8_t staminaDepletionRate;
	uint8_t staminaRecoveryRate;	
	uint8_t totalHunger;			// Certain NPCs can have a larger hunger bar, storing more food			
	uint8_t hungerDepletionRate;
	uint8_t hungerRecoveryBasic;	// How much hunger is restored. Used for basic food sources, like grass or flowers. 
	uint8_t hungerRecoveryImproved;	// How much hunger is restored. Used for improved food sources, like other animals.
	uint8_t eatingSpeedRateBasic;	// How fast the animal will consume the basic food sources
	uint8_t eatingSpeedRateImproved;// How fast the animal will consume the improved food sources

	uint8_t restAfterFoodBasic;		// How long should the NPC rest after eating a basic food source.
	uint8_t restAfterFoodImproved;	// How long should the NPC rest after eating an improved food source.

	uint8_t mealsUntilRestIsNeeded; // After how many meals the NPC should rest.

	int foodPouch;					// How much food is stored in the food pouch. This is an overflow that an NPC can store and use as a reserve or for trading.

	uint8_t desireToHoardFood;		// How much should the NPC want to keep collecting food after the total hunger is satisfied

	uint8_t chaseDesire;			// How willing is the animal to chase other targets (animal food sources)
	
	uint16_t awarenessRadius;		// The sphere radius that determines which NPCs or objects are being tracked by the NPC
	
	uint16_t fleeingRadius;			// The radius around the NPC from which to pick a point to run towards when being chased.

	uint8_t reactionSpeed;			// How fast should the NPC react to the environment. This determines how quickly the should intrerupt their current action and replace it with another (for example fleeing from an enemy).

	uint8_t willignessToBecomeAlly; // How willing is the NPC to become someone else's ally and become a pack

	uint8_t desireToRecruitAllies;	// How willing is the animal to trade food from the food pouch to attempt to recruit allies
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
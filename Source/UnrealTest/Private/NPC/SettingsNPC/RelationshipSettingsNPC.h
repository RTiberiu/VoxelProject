#pragma once

#include <cstdint>
#include <type_traits>

// Setting the bit for each of the type of animal
enum class AnimalType : uint16_t {
    RedPanda    = 1 << 0,
    Tiger       = 1 << 1,
    Tapir       = 1 << 2,
    Sloth       = 1 << 3,
    Cobra       = 1 << 4,
    Bat         = 1 << 5,
    Peacock     = 1 << 6,
    Gorilla     = 1 << 7,
    Panda       = 1 << 8,
    None        = 0
};

struct AnimalRelationships {
	AnimalType FoodType;
	AnimalType Allies;
	AnimalType Enemies;
};

using enum AnimalType;

// Overloading the | operator to work directly with the AnimalType enum
constexpr AnimalType operator|(AnimalType lhs, AnimalType rhs) noexcept {
	using T = std::underlying_type_t<AnimalType>;
	return static_cast<AnimalType>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

// Overloading the & operator for bitwise operations between AnimalType objects
constexpr AnimalType operator&(AnimalType lhs, AnimalType rhs) noexcept {
	using T = std::underlying_type_t<AnimalType>;
	return static_cast<AnimalType>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

// Tiger settings
constexpr AnimalType TigerFoodType = RedPanda | Sloth | Tapir;
constexpr AnimalType TigerAllies = Gorilla | Panda | Tiger;
constexpr AnimalType TigerEnemies = None;

// Tapir settings
constexpr AnimalType TapirFoodType = None;
constexpr AnimalType TapirAllies = RedPanda | Sloth | Bat | Gorilla | Panda | Peacock;
constexpr AnimalType TapirEnemies = Tiger;

// Sloth settings
constexpr AnimalType SlothFoodType = None;
constexpr AnimalType SlothAllies = RedPanda | Tapir | Bat | Gorilla | Panda | Peacock;
constexpr AnimalType SlothEnemies = Tiger;

// Cobra settings
constexpr AnimalType CobraFoodType = Bat | Peacock | RedPanda;
constexpr AnimalType CobraAllies = Tapir | Sloth | Gorilla | Panda | Tiger;
constexpr AnimalType CobraEnemies = None;

// Bat settings
constexpr AnimalType BatFoodType = None;
constexpr AnimalType BatAllies = Peacock | Tapir | Gorilla | Panda | Sloth;
constexpr AnimalType BatEnemies = Cobra;

// Peacock settings
constexpr AnimalType PeacockFoodType = None;
constexpr AnimalType PeacockAllies = Bat | Tapir | Gorilla | Panda | Sloth;
constexpr AnimalType PeacockEnemies = Cobra;

// Gorilla settings
constexpr AnimalType GorillaFoodType = None;
constexpr AnimalType GorillaAllies = Tiger | Tapir | RedPanda | Sloth | Cobra | Bat | Peacock | Panda;
constexpr AnimalType GorillaEnemies = None;

// Panda settings
constexpr AnimalType PandaFoodType = None;
constexpr AnimalType PandaAllies = Tiger | Tapir | RedPanda | Sloth | Cobra | Bat | Peacock | Gorilla;
constexpr AnimalType PandaEnemies = None;

extern TMap<AnimalType, FString> TypeToName;
extern TMap<AnimalType, AnimalRelationships> AnimalsRelationships;


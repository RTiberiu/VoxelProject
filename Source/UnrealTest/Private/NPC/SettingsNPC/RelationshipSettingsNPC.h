#pragma once

#include <cstdint>
#include <type_traits>

enum class AnimalType : uint16_t {
	RedPanda,
	Tiger,
	Tapir,
	Sloth,
	Cobra,
	Bat,
	Peacock,
	Gorilla,
	Panda,
	None
};

using enum AnimalType;

// Overloading the | operator to work directly with the AnimalType enum
constexpr AnimalType operator|(AnimalType lhs, AnimalType rhs) noexcept {
	using T = std::underlying_type_t<AnimalType>;
	return static_cast<AnimalType>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

// Tiger settings
constexpr AnimalType TigerFoodType = RedPanda | Sloth | Tapir;
constexpr AnimalType TigerAllies = Gorilla | Panda;
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
constexpr AnimalType CobraAllies = Peacock | Tapir | Gorilla | Panda | Sloth;
constexpr AnimalType CobraEnemies = Cobra;

// Peacock settings
constexpr AnimalType PeacockFoodType = None;
constexpr AnimalType PeacockAllies = Bat | Tapir | Gorilla | Panda | Sloth;
constexpr AnimalType PeacockEnemies = Cobra;

// Gorilla settings
constexpr AnimalType GorillaFoodType = None;
constexpr AnimalType GorillaAllies = Tiger | Tapir | RedPanda | Sloth | Cobra | Bat | Peacock | Panda;
constexpr AnimalType GorillaEnemies = None;

// Panda settings
constexpr AnimalType GorillaFoodType = None;
constexpr AnimalType GorillaAllies = Tiger | Tapir | RedPanda | Sloth | Cobra | Bat | Peacock | Gorilla;
constexpr AnimalType GorillaEnemies = None;

// TODO CONTINUE AND ADJUST THE RELATIONSHIPS 

#include "RelationshipSettingsNPC.h"


TMap<AnimalType, AnimalRelationships> AnimalsRelationships{
	{ Tiger,   { TigerFoodType,   TigerAllies,   TigerEnemies   } },
	{ Tapir,   { TapirFoodType,   TapirAllies,   TapirEnemies   } },
	{ Sloth,   { SlothFoodType,   SlothAllies,   SlothEnemies   } },
	{ Cobra,   { CobraFoodType,   CobraAllies,   CobraEnemies   } },
	{ Bat,     { BatFoodType,     BatAllies,     BatEnemies     } },
	{ Peacock, { PeacockFoodType, PeacockAllies, PeacockEnemies } },
	{ Gorilla, { GorillaFoodType, GorillaAllies, GorillaEnemies } },
	{ Panda,   { PandaFoodType,   PandaAllies,   PandaEnemies   } },
	{ RedPanda, {RedPandaFoodType, RedPandaAllies, RedPandaEnemies}}
};

TMap<AnimalType, FString> TypeToName = {
	{ Tiger,   "Tiger"   },
	{ Tapir,   "Tapir"   },
	{ Sloth,   "Sloth"   },
	{ Cobra,   "Cobra"   },
	{ Bat,     "Bat"     },
	{ Peacock, "Peacock" },
	{ Gorilla, "Gorilla" },
	{ Panda,   "Panda"   },
	{ RedPanda, "RedPanda"}
};

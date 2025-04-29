#pragma once  

#include "CoreMinimal.h"  
#include "..\..\Chunks\DataStructures\VoxelObjectLocationData.h"  
#include "..\..\NPC\SettingsNPC\RelationshipSettingsNPC.h"  

// ---------------------------------------------------------------------------
// This is used to store various testing scenarios, storing NPCs positions  
// and/or vegetation positions to spawn in the world.  
//  
// They are supposed to showcase a certain behaviour, things like  
// pathfinding, collision, NPC actions, NPC notifications, etc.  
//  
// Using these testing scenarios will replace the default world generation  
// ---------------------------------------------------------------------------

// Other positions I could use 
//{ FVoxelObjectLocationData(FVector(560, 560, 6130), FIntPoint(0, 1)), AnimalType::Tiger },
//{ FVoxelObjectLocationData(FVector(560, 210, 6130), FIntPoint(0, 1)), AnimalType::Tiger },
//{ FVoxelObjectLocationData(FVector(740, 920, 6130), FIntPoint(0, 1)), AnimalType::Peacock },
//{ FVoxelObjectLocationData(FVector(560, 1220, 6130), FIntPoint(0, 1)), AnimalType::Tiger },
//{ FVoxelObjectLocationData(FVector(1340, 210, 6430), FIntPoint(0, 1)), AnimalType::Peacock },
//{ FVoxelObjectLocationData(FVector(970, 330, 6130), FIntPoint(0, 1)), AnimalType::Tiger },
//{ FVoxelObjectLocationData(FVector(1100, 270, 6130), FIntPoint(0, 1)), AnimalType::Peacock },
//{ FVoxelObjectLocationData(FVector(1160, 270, 6190), FIntPoint(0, 1)), AnimalType::Peacock }

enum ConfigToRun {
    NotificationAttackNpc,
    NotificationAttackFoodSource
};

struct TestConfigParameters {
    TArray<TPair<FVoxelObjectLocationData, AnimalType>> NpcPositions;
    TArray<FVoxelObjectLocationData> Grass;
    TArray<FVoxelObjectLocationData> Flowers;
};

namespace TestingConfig {
    // Use only ShouldAttackNpc() and ShouldRelax()
    inline TestConfigParameters GetNotificationAttackNpcTest() {
        return TestConfigParameters{
            // NpcPositions
            {
                { FVoxelObjectLocationData(FVector(560, 440, 6130), FIntPoint(0, 1)), AnimalType::Tiger },
                { FVoxelObjectLocationData(FVector(560, 930, 6070), FIntPoint(0, 1)), AnimalType::Peacock },
                { FVoxelObjectLocationData(FVector(970, -30, 6130), FIntPoint(0, 1)), AnimalType::Tiger }
            },
            // Grass (none)
            {},
            // Flowers (none)
            {}
        };
    }

    // Use only ShouldEatBasicFoodSource() and ShouldRelax()
    inline TestConfigParameters GetNotificationAttackFoodTest() {
        return TestConfigParameters{
            // NpcPositions
            {
                { FVoxelObjectLocationData(FVector(560, 440, 6130), FIntPoint(0, 1)), AnimalType::Tiger },
                { FVoxelObjectLocationData(FVector(970, -30, 6130), FIntPoint(0, 1)), AnimalType::Tiger }
            },
            // Grass
            {
                FVoxelObjectLocationData(FVector(515, 705, 6060), FIntPoint(0, 0))
            },
            // Flowers (none)
            {}
        };
    }


    // TODO Create more scenarios like this and spawn the objects.

}

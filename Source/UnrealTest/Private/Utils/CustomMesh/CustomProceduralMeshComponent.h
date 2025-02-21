#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "CustomProceduralMeshComponent.generated.h"

enum MeshType {
    Grass,
    Flower
};

UCLASS()
class UCustomProceduralMeshComponent : public UProceduralMeshComponent {
    GENERATED_BODY()

public:
    FIntPoint ObjectWorldCoords;
    MeshType MeshType;
};

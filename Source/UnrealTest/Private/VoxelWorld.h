#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Voxel.h"
#include "BinaryGreedyMesher.h"
#include "VoxelWorld.generated.h"

UCLASS()
class AVoxelWorld : public AActor {
    GENERATED_BODY()

public:
    AVoxelWorld();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    TArray<FVoxel> Voxels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    int SizeX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    int SizeY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    int SizeZ;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    UMaterial* VoxelMaterial;

    UFUNCTION(BlueprintCallable, Category = "Voxel")
    void GenerateMesh();

    UFUNCTION(BlueprintCallable, Category = "Voxel")
    void InitializeVoxelData();

private:
    void CreateQuad(const FQuad& Quad);
    UStaticMesh* CubeMesh;
    BinaryGreedyMesher Mesher;
};

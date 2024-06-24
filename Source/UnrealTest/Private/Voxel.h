#pragma once

#include "Voxel.generated.h"

USTRUCT(BlueprintType)
struct FVoxel {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSolid;

    // Additional data like color, texture, etc. can be added here
};

USTRUCT(BlueprintType)
struct FQuad {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 X;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Y;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Z;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Axis; // 0 for XY, 1 for YZ, 2 for XZ
};
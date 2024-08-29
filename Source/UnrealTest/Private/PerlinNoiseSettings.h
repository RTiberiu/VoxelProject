// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PerlinNoiseSettings.generated.h"

UCLASS()
class APerlinNoiseSettings : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APerlinNoiseSettings();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<int> Octaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<float> Frequencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<int32> Amplitudes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<float> Lacunarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<float> Gain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<float> WeightedStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    bool changedSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float noiseFrequency3D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float squashingFactor;
    
    // Domain warp settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<int> DomainWarpAmp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<float> DomainWarpFrequencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<int> DomainWarpOctaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<float> DomainWarpLacunarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<float> DomainWarpGain;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

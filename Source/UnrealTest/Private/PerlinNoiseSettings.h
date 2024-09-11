// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PerlinNoiseSettings.generated.h"

struct BiomeNoiseSettings {
	TArray<int> Octaves;
	TArray<float> Frequencies;
	TArray<int> Amplitudes;
	TArray<float> Lacunarity;
	TArray<float> Gain;
	TArray<float> WeightedStrength;
	TArray<int> DomainWarpAmp;
	TArray<float> DomainWarpFrequencies;
	TArray<int> DomainWarpOctaves;
	TArray<float> DomainWarpLacunarity;
	TArray<float> DomainWarpGain;
};

UCLASS()
class APerlinNoiseSettings : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APerlinNoiseSettings();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    bool changedSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float noiseFrequency3D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float squashingFactor;

    TArray<BiomeNoiseSettings> biomes;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

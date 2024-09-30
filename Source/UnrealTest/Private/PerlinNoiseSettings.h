// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PerlinNoiseSettings.generated.h"

USTRUCT(BlueprintType)
struct FNoiseMapSettings {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map")
		int Octaves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map") 
		float Frequencies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map")
		int Amplitudes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map") 
		float Lacunarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map")
		float Gain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map") 
		float WeightedStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map") 
		int DomainWarpAmp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map") 
		float DomainWarpFrequencies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map") 
		int DomainWarpOctaves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map") 
		float DomainWarpLacunarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map")
		float DomainWarpGain;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
	int biomeIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Settings")
	TArray<FNoiseMapSettings> noiseMapSettings;
    // TArray<NoiseMapSettings> noiseMapSettings;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NoiseMapSettings.h"
#include "GameFramework/Actor.h"
#include "PerlinNoiseSettings.generated.h"

UCLASS()
class APerlinNoiseSettings : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APerlinNoiseSettings();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    bool changedSettings;

	UFUNCTION(CallInEditor, Category = "Terrain Generation")
	void OnChangedSettings();

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

	void BindInputToRespawnTerrain();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

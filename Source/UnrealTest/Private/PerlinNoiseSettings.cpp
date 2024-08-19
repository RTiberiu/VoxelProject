// Fill out your copyright notice in the Description page of Project Settings.

#include "PerlinNoiseSettings.h"

// Sets default values
APerlinNoiseSettings::APerlinNoiseSettings() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Flat-ish Valleys biome noise 
	//Octaves = { 2, 3, 4};
	//Frequencies = { 0.007f, 0.01f, 0.003f };
	//Amplitudes = { 20, 15, 15 };
	//Lacunarity = { 2.0f, 2.0f, 2.0f };
	//Gain = { 0.5f, 0.5f, 0.5f };
	//WeightedStrength = { -1.7f, 0.0f, -0.7f };

	// Mountain biome noise
	Octaves = { 3, 3, 5 };
	Frequencies = { 0.003f, 0.006f, 0.015f };
	Amplitudes = { 50, 90, 40 };
	Lacunarity = { 2.0f, 2.0f, 2.0f };
	Gain = { 0.5f, 0.5f, 0.5f };
	WeightedStrength = { 0.0f, 0.0f, -0.7f };

	noiseFrequency3D = 0.012;
	squashingFactor = 0.004;

	changedSettings = false;
}

// Called when the game starts or when spawned
void APerlinNoiseSettings::BeginPlay() {
	Super::BeginPlay();

}

// Called every frame
void APerlinNoiseSettings::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

}


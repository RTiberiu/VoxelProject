// Fill out your copyright notice in the Description page of Project Settings.

#include "PerlinNoiseSettings.h"

// Sets default values
APerlinNoiseSettings::APerlinNoiseSettings() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	FNoiseMapSettings continentalness;
	continentalness.Octaves = 4;
	continentalness.Frequencies = 0.002f;
	continentalness.Amplitudes = 50;
	continentalness.Lacunarity = 1.0f;
	continentalness.Gain = 0.7f;
	continentalness.WeightedStrength = -0.2f;
	continentalness.DomainWarpAmp = 80;
	continentalness.DomainWarpFrequencies = 0.005f;
	continentalness.DomainWarpOctaves = 3;
	continentalness.DomainWarpLacunarity = 2.2f;
	continentalness.DomainWarpGain = 0.3f;
		
	FNoiseMapSettings erosion;
	erosion.Octaves = 2;
	erosion.Frequencies = 0.005f;
	erosion.Amplitudes = 0;
	erosion.Lacunarity = 2.0f;
	erosion.Gain = 0.5f;
	erosion.WeightedStrength = -1.3f;
	erosion.DomainWarpAmp = 80;
	erosion.DomainWarpFrequencies = 0.03f;
	erosion.DomainWarpOctaves = 3;
	erosion.DomainWarpLacunarity = 1.2f;
	erosion.DomainWarpGain = 1.0f;

	FNoiseMapSettings peaksAndValleys;
	peaksAndValleys.Octaves = 4;
	peaksAndValleys.Frequencies = 0.01f;
	peaksAndValleys.Amplitudes = 0;
	peaksAndValleys.Lacunarity = 1.0f;
	peaksAndValleys.Gain = 1.2f;
	peaksAndValleys.WeightedStrength = -1.5f;
	peaksAndValleys.DomainWarpAmp = 10;
	peaksAndValleys.DomainWarpFrequencies = 0.041f;
	peaksAndValleys.DomainWarpOctaves = 3;
	peaksAndValleys.DomainWarpLacunarity = 0.66f;
	peaksAndValleys.DomainWarpGain = 0.74f;

	biomeIndex = 2; 

	noiseMapSettings.Add(continentalness);
	noiseMapSettings.Add(erosion);
	noiseMapSettings.Add(peaksAndValleys);
	
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


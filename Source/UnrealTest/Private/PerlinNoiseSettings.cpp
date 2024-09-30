// Fill out your copyright notice in the Description page of Project Settings.

#include "PerlinNoiseSettings.h"

// Sets default values
APerlinNoiseSettings::APerlinNoiseSettings() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	FNoiseMapSettings continentalness;
	continentalness.Octaves = 3;
	continentalness.Frequencies = 0.005f;
	continentalness.Amplitudes = 50;
	continentalness.Lacunarity = 2.0f;
	continentalness.Gain = 0.66f;
	continentalness.WeightedStrength = -0.15f;
	continentalness.DomainWarpAmp = 47;
	continentalness.DomainWarpFrequencies = 0.037f;
	continentalness.DomainWarpOctaves = 3;
	continentalness.DomainWarpLacunarity = 2.25f;
	continentalness.DomainWarpGain = 0.25f;
		
	FNoiseMapSettings erosion;
	erosion.Octaves = 2;
	erosion.Frequencies = 0.007f;
	erosion.Amplitudes = 20;
	erosion.Lacunarity = 2.0f;
	erosion.Gain = 0.5f;
	erosion.WeightedStrength = -1.3f;
	erosion.DomainWarpAmp = 80;
	erosion.DomainWarpFrequencies = 0.03f;
	erosion.DomainWarpOctaves = 3;
	erosion.DomainWarpLacunarity = 1.2f;
	erosion.DomainWarpGain = 1.0f;

	FNoiseMapSettings peaksAndValleys;
	peaksAndValleys.Octaves = 3;
	peaksAndValleys.Frequencies = 0.004f;
	peaksAndValleys.Amplitudes = 80;
	peaksAndValleys.Lacunarity = 2.02f;
	peaksAndValleys.Gain = 0.4f;
	peaksAndValleys.WeightedStrength = -1.2f;
	peaksAndValleys.DomainWarpAmp = 80;
	peaksAndValleys.DomainWarpFrequencies = 0.03f;
	peaksAndValleys.DomainWarpOctaves = 3;
	peaksAndValleys.DomainWarpLacunarity = 1.2f;
	peaksAndValleys.DomainWarpGain = 1.0f;

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


// Fill out your copyright notice in the Description page of Project Settings.

#include "PerlinNoiseSettings.h"

// Sets default values
APerlinNoiseSettings::APerlinNoiseSettings() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BiomeNoiseSettings flatBiome;
	flatBiome.Octaves = { 4, 4, 4 };
	flatBiome.Frequencies = { 0.003f, 0.01f, 0.01f };
	flatBiome.Amplitudes = { 20, 10, 10 };
	flatBiome.Lacunarity = { 2.0f, 2.0f, 2.0f };
	flatBiome.Gain = { 0.5f, 0.5f, 0.5f };
	flatBiome.WeightedStrength = { 0.0f, 0.0f, 0.0f };
	flatBiome.DomainWarpAmp = { 60, 60, 60 };
	flatBiome.DomainWarpFrequencies = { 0.05f, 0.05f, 0.05f };
	flatBiome.DomainWarpOctaves = { 3, 3, 3 };
	flatBiome.DomainWarpLacunarity = { 1.0f, 1.0f, 1.0f};
	flatBiome.DomainWarpGain = { 1.0f, 1.0f, 1.0f };
		
	BiomeNoiseSettings valleysBiome;
	valleysBiome.Octaves = { 2, 3, 4 };
	valleysBiome.Frequencies = { 0.007f, 0.01f, 0.003f };
	valleysBiome.Amplitudes = { 20, 15, 15 };
	valleysBiome.Lacunarity = { 2.0f, 2.0f, 2.0f };
	valleysBiome.Gain = { 0.5f, 0.5f, 0.5f };
	valleysBiome.WeightedStrength = { -1.3f, 0.0f, -0.7f };
	valleysBiome.DomainWarpAmp = { 80, 60, 100 };
	valleysBiome.DomainWarpFrequencies = { 0.03f, 0.06f, 0.06f };
	valleysBiome.DomainWarpOctaves = { 3, 3, 3 };
	valleysBiome.DomainWarpLacunarity = { 1.2f, 1.0f, 0.8f };
	valleysBiome.DomainWarpGain = { 1.0f, 1.0f, 1.0f };

	BiomeNoiseSettings mountainBiome;
	mountainBiome.Octaves = { 3, 3, 5 };
	mountainBiome.Frequencies = { 0.004f, 0.006f, 0.015f };
	mountainBiome.Amplitudes = { 80, 70, 15 };
	mountainBiome.Lacunarity = { 2.02f, 2.0f, 2.0f };
	mountainBiome.Gain = { 0.4f, 0.5f, 0.5f };
	mountainBiome.WeightedStrength = { -1.2f, 0.0f, -0.7f };
	mountainBiome.DomainWarpAmp = { 80, 60, 100 };
	mountainBiome.DomainWarpFrequencies = { 0.03f, 0.06f, 0.06f };
	mountainBiome.DomainWarpOctaves = { 3, 3, 3 };
	mountainBiome.DomainWarpLacunarity = { 1.2f, 1.0f, 0.8f };
	mountainBiome.DomainWarpGain = { 1.0f, 1.0f, 1.0f };

	biomeIndex = 2; 

	biomes.Add(flatBiome);
	biomes.Add(valleysBiome);
	biomes.Add(mountainBiome);

	// Flat-ish Valleys biome noise - DONE
	/*Octaves = { 2, 3, 4 };
	Frequencies = { 0.007f, 0.01f, 0.003f };
	Amplitudes = { 20, 15, 15 };
	Lacunarity = { 2.0f, 2.0f, 2.0f };
	Gain = { 0.5f, 0.5f, 0.5f };
	WeightedStrength = { -1.3f, 0.0f, -0.7f };

	DomainWarpAmp = { 80, 60, 100 };
	DomainWarpFrequencies = { 0.03f, 0.06f, 0.06f };
	DomainWarpOctaves = { 3, 3, 3 };
	DomainWarpLacunarity = { 1.2f, 1.0f, 0.8f };
	DomainWarpGain = { 1.0f, 1.0f, 1.0f };*/

	// Mountain biome noise - DONE
	/*Octaves = { 3, 3, 5 };
	Frequencies = { 0.004f, 0.006f, 0.015f };
	Amplitudes = { 80, 70, 15 };
	Lacunarity = { 2.02f, 2.0f, 2.0f };
	Gain = { 0.4f, 0.5f, 0.5f };
	WeightedStrength = { -1.2f, 0.0f, -0.7f };

	DomainWarpAmp = {80, 60, 100};
	DomainWarpFrequencies = { 0.03f, 0.06f, 0.06f};
	DomainWarpOctaves = {3, 3, 3};
	DomainWarpLacunarity = { 1.2f, 1.0f, 0.8f };
	DomainWarpGain = { 1.0f, 1.0f, 1.0f };*/

	// World biome noise (Euclidian Sq, Distance 2 Div)
	/*WorldFrequency = 0.005f;
	WorldOctaves = 3;
	WorldLacunarity = 0.5f;
	WorldGain = 0.01f;
	WorldWeightedStrength = 0.0f;

	WorldJitter = 1.13f;
	WorldAmplitude = 65.0f;
	WorldFrequency = 0.011f;*/

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


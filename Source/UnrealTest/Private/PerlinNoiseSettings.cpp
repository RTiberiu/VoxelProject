// Fill out your copyright notice in the Description page of Project Settings.

#include "PerlinNoiseSettings.h"

// Sets default values
APerlinNoiseSettings::APerlinNoiseSettings() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	OctaveFrequencies = { 0.003f, 0.005f, 0.008f };
	Amplitudes = { 30, 80, 15 };
	Lacunarity = { 1.37f, 2.0f, 2.0f };
	Gain = { 5.0f, 0.8f, 0.2f };
	WeightedStrength = { 0.0f, 0.0f, 0.2f };

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


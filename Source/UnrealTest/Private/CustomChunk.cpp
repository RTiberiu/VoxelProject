// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomChunk.h"

#include "FastNoiseLite.h"

// Sets default values
ACustomChunk::ACustomChunk() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	/* TODO Get the perlin noise
	*  TODO Write a function similar to the Three.js
	* and get the coordinates for each cube.
	* TODO Find the best way to store these coordinates for a chunk.
	* TODO Function to spawn cube at location. (need to spawn only visible faces)
	*/

}

// Called when the game starts or when spawned
void ACustomChunk::BeginPlay() {
	Super::BeginPlay();

}

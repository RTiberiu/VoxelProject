// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomChunkWorld.generated.h"

UCLASS()
class ACustomChunkWorld : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACustomChunkWorld();

	UPROPERTY(EditAnywhere, Category = "Custom Chunk World")
	TSubclassOf<AActor> chunk;

	UPROPERTY(EditAnywhere, Category = "Custom Chunk World")
	int drawDistance{ 5 };

	UPROPERTY(EditAnywhere, Category = "Custom Chunk World")
	int chunkSize{ 64 }; // 32

	UPROPERTY(EditAnywhere, Category = "Custom Chunk World")
	int UnrealScale{ 100 };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};

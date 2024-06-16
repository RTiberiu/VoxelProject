// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomChunk.generated.h"

UCLASS()
class ACustomChunk : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACustomChunk();

	UPROPERTY(EditAnywhere, Category = "Custom Chunk")
	int chunkSize{ 32 };

	UPROPERTY(EditAnywhere, Category = "Custom Chunk")
	int chunkScale{ 1 };

private:
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


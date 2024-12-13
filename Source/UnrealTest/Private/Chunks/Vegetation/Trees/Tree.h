#pragma once

#include "ProceduralMeshComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tree.generated.h"

UCLASS()
class ATree : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATree();

	~ATree();

private:
	bool hasCollision;

	TObjectPtr<UProceduralMeshComponent> Mesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};

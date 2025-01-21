# pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include <GameFramework/FloatingPawnMovement.h>

#include "BasicNPC.generated.h"

class UWorldTerrainSettings;

UCLASS()
class ABasicNPC : public APawn {
	GENERATED_BODY()

public:
	ABasicNPC();
	~ABasicNPC();

	void SetNPCWorldLocation(FIntPoint InNPCLocation);

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
private:
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	void spawnNPC();
	void buildAnimationsList();

	FIntPoint NPCLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* SkeletalMesh;

	UFloatingPawnMovement* FloatingMovement;

	static constexpr int animationChangeAfterFrames = 180;
	int frameCounter = 0;

	TArray<FString> Animations;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;



};
#include "Tree.h"

// Sets default values
ATree::ATree() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");

	Mesh->SetCastShadow(false);

	// By default the tree shouldn't have collision
	hasCollision = false;

	// Set mesh as root
	//SetRootComponent(Mesh);
}

ATree::~ATree() {
}

void ATree::BeginPlay() {
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Tree message"));
}

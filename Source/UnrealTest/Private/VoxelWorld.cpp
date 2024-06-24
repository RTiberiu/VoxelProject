#include "VoxelWorld.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AVoxelWorld::AVoxelWorld() {
    PrimaryActorTick.bCanEverTick = false;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshObj(TEXT("/Engine/BasicShapes/Cube.Cube"));
    CubeMesh = CubeMeshObj.Object;

    // Initialize default voxel data
    SizeX = 10;
    SizeY = 10;
    SizeZ = 10;
    Voxels.SetNum(SizeX * SizeY * SizeZ);

    // Initialize voxel material
    static ConstructorHelpers::FObjectFinder<UMaterial> VoxelMatObj(TEXT("/Game/StarterContent/Materials/M_Brick_Clay_New.M_Brick_Clay_New"));
    VoxelMaterial = VoxelMatObj.Object;

    // Fill voxel data
    InitializeVoxelData();
}

void AVoxelWorld::BeginPlay() {
    Super::BeginPlay();
    GenerateMesh();
}

void AVoxelWorld::GenerateMesh() {
    TArray<FQuad> Quads = Mesher.Mesh(Voxels, SizeX, SizeY, SizeZ);

    for (const FQuad& Quad : Quads) {
        CreateQuad(Quad);
    }
}

void AVoxelWorld::InitializeVoxelData() {
    for (int Z = 0; Z < SizeZ; ++Z) {
        for (int Y = 0; Y < SizeY; ++Y) {
            for (int X = 0; X < SizeX; ++X) {
                int Index = X + Y * SizeX + Z * SizeX * SizeY;
                Voxels[Index].bSolid = (X + Y + Z) % 2 == 0;  // Example pattern
            }
        }
    }
}

void AVoxelWorld::CreateQuad(const FQuad& Quad) {
    FVector Position;
    FVector Scale;
    FVector Offset;

    switch (Quad.Axis) {
    case 0: // XY
        Position = FVector(Quad.X + Quad.Width / 2.0f, Quad.Y + Quad.Height / 2.0f, Quad.Z);
        Scale = FVector(Quad.Width, Quad.Height, 1);
        Offset = FVector(0.5f, 0.5f, 0);
        break;
    case 1: // YZ
        Position = FVector(Quad.X, Quad.Y + Quad.Width / 2.0f, Quad.Z + Quad.Height / 2.0f);
        Scale = FVector(1, Quad.Width, Quad.Height);
        Offset = FVector(0, 0.5f, 0.5f);
        break;
    case 2: // XZ
        Position = FVector(Quad.X + Quad.Width / 2.0f, Quad.Y, Quad.Z + Quad.Height / 2.0f);
        Scale = FVector(Quad.Width, 1, Quad.Height);
        Offset = FVector(0.5f, 0, 0.5f);
        break;
    }

    Position -= Offset;

    UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this);
    MeshComponent->SetStaticMesh(CubeMesh);
    MeshComponent->SetMaterial(0, VoxelMaterial);
    MeshComponent->SetWorldLocation(Position);
    MeshComponent->SetWorldScale3D(Scale);
    MeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
    MeshComponent->RegisterComponent();
}

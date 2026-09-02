#include "World/CricketPracticePitch.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ACricketPracticePitch::ACricketPracticePitch()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

    if (!CubeFinder.Succeeded() || !CylinderFinder.Succeeded())
    {
        return;
    }

    // 20.12m cricket strip and a simple 130m-diameter playable outfield proxy.
    CreateMeshPart(TEXT("Outfield"), CubeFinder.Object, FVector(0, 0, -28), FVector(130.0f, 130.0f, 0.18f), TEXT("Outfield"));
    CreateMeshPart(TEXT("Pitch"), CubeFinder.Object, FVector(0, 0, -4), FVector(20.12f, 3.05f, 0.04f), TEXT("Pitch"));

    const float StumpX[2] = { 850.0f, -850.0f };
    int32 Index = 0;
    for (float X : StumpX)
    {
        for (int32 S = -1; S <= 1; ++S)
        {
            const FName Name(*FString::Printf(TEXT("Stump_%d"), Index++));
            CreateMeshPart(Name, CylinderFinder.Object, FVector(X, S * 11.0f, 35.5f), FVector(0.022f, 0.022f, 0.711f), TEXT("Stump"));
        }
    }
}

UStaticMeshComponent* ACricketPracticePitch::CreateMeshPart(const FName Name, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale, const FName Tag)
{
    UStaticMeshComponent* Component = CreateDefaultSubobject<UStaticMeshComponent>(Name);
    Component->SetupAttachment(SceneRoot);
    Component->SetStaticMesh(Mesh);
    Component->SetRelativeLocation(Location);
    Component->SetRelativeScale3D(Scale);
    Component->SetMobility(EComponentMobility::Static);
    Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Component->SetCollisionObjectType(ECC_WorldStatic);
    Component->ComponentTags.Add(Tag);
    return Component;
}

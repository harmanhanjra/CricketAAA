#include "Characters/BatterCharacter.h"
#include "Batting/BattingComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ABatterCharacter::ABatterCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    BattingComponent = CreateDefaultSubobject<UBattingComponent>(TEXT("BattingComponent"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

    BodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyVisual"));
    BodyVisual->SetupAttachment(GetRootComponent());
    BodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BodyVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
    BodyVisual->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.95f));
    if (CylinderMesh.Succeeded()) BodyVisual->SetStaticMesh(CylinderMesh.Object);

    BatVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BatVisual"));
    BatVisual->SetupAttachment(GetRootComponent());
    BatVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BatVisual->SetRelativeLocation(FVector(-55.0f, 35.0f, 40.0f));
    BatVisual->SetRelativeRotation(FRotator(0.0f, 10.0f, 15.0f));
    BatVisual->SetRelativeScale3D(FVector(0.9f, 0.08f, 0.04f));
    if (CubeMesh.Succeeded()) BatVisual->SetStaticMesh(CubeMesh.Object);
}

#include "Characters/BowlerCharacter.h"
#include "Bowling/BowlingComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ABowlerCharacter::ABowlerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    BowlingComponent = CreateDefaultSubobject<UBowlingComponent>(TEXT("BowlingComponent"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    BodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyVisual"));
    BodyVisual->SetupAttachment(GetRootComponent());
    BodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BodyVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
    BodyVisual->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.95f));
    if (CylinderMesh.Succeeded()) BodyVisual->SetStaticMesh(CylinderMesh.Object);
}

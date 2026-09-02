#include "Characters/FielderCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AFielderCharacter::AFielderCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    GetCharacterMovement()->MaxWalkSpeed = 720.0f;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    BodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyVisual"));
    BodyVisual->SetupAttachment(GetRootComponent());
    BodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BodyVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
    BodyVisual->SetRelativeScale3D(FVector(0.30f, 0.30f, 0.90f));
    if (CylinderMesh.Succeeded()) BodyVisual->SetStaticMesh(CylinderMesh.Object);
}

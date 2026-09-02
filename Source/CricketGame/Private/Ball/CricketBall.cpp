#include "Ball/CricketBall.h"
#include "Ball/BallPhysicsComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ACricketBall::ACricketBall()
{
    PrimaryActorTick.bCanEverTick = true;

    BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
    SetRootComponent(BallMesh);
    BallMesh->SetSimulatePhysics(true);
    BallMesh->SetEnableGravity(true);
    BallMesh->SetNotifyRigidBodyCollision(true);
    BallMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    BallMesh->BodyInstance.bUseCCD = true;
    BallMesh->SetMassOverrideInKg(NAME_None, 0.163f, true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        BallMesh->SetStaticMesh(SphereMesh.Object);
        BallMesh->SetWorldScale3D(FVector(0.072f));
    }

    BallPhysics = CreateDefaultSubobject<UBallPhysicsComponent>(TEXT("BallPhysics"));
    BallPhysics->AttachPhysicsBody(BallMesh);

    BallMesh->OnComponentHit.AddDynamic(this, &ACricketBall::HandleHit);
}

void ACricketBall::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bBallLive)
    {
        return;
    }

    LiveSeconds += DeltaSeconds;

    if (!bBoundaryReported && GetActorLocation().Size2D() >= BoundaryRadiusCm && bWasHitByBat)
    {
        bBoundaryReported = true;
        bBallLive = false;
        BallPhysics->SetActiveFlight(false);
        OnBoundary.Broadcast(bBouncedAfterBat ? 4 : 6);
        return;
    }

    const float Speed = BallMesh->GetPhysicsLinearVelocity().Size();
    if (LiveSeconds > 1.0f && Speed < 90.0f)
    {
        SlowSeconds += DeltaSeconds;
    }
    else
    {
        SlowSeconds = 0.0f;
    }

    if (SlowSeconds > 0.8f || LiveSeconds > 12.0f || GetActorLocation().Z < -500.0f)
    {
        bBallLive = false;
        BallPhysics->SetActiveFlight(false);
        OnBallDead.Broadcast();
    }
}

void ACricketBall::LaunchDelivery(const FVector& StartLocation, const FVector& InitialVelocityCm, const FVector& SpinRadPerSec, float SwingAmount)
{
    SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);
    BallMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
    BallMesh->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
    BallMesh->SetSimulatePhysics(true);
    BallMesh->SetEnableGravity(true);
    BallMesh->SetPhysicsLinearVelocity(InitialVelocityCm);

    BallPhysics->SetSpinRadPerSec(SpinRadPerSec);
    BallPhysics->SetSwingAmount(SwingAmount);
    BallPhysics->SetActiveFlight(true);

    bBallLive = true;
    bWasHitByBat = false;
    bBouncedAfterBat = false;
    bBoundaryReported = false;
    LiveSeconds = 0.0f;
    SlowSeconds = 0.0f;
}

void ACricketBall::ApplyBatContact(const FVector& ExitVelocityCm, const FVector& NewSpinRadPerSec)
{
    if (!bBallLive)
    {
        return;
    }

    BallMesh->SetPhysicsLinearVelocity(ExitVelocityCm);
    BallPhysics->SetSpinRadPerSec(NewSpinRadPerSec);
    BallPhysics->SetSwingAmount(0.0f);
    bWasHitByBat = true;
    bBouncedAfterBat = false;
}

void ACricketBall::ResetBall(const FVector& NewLocation)
{
    bBallLive = false;
    BallPhysics->SetActiveFlight(false);
    BallMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
    BallMesh->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
    BallMesh->SetSimulatePhysics(false);
    SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ACricketBall::FieldBall()
{
    if (!bBallLive)
    {
        return;
    }

    bBallLive = false;
    BallPhysics->SetActiveFlight(false);
    BallMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
    OnBallDead.Broadcast();
}

void ACricketBall::HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!bBallLive || !OtherComp)
    {
        return;
    }

    if (OtherComp->ComponentHasTag(TEXT("Stump")))
    {
        bBallLive = false;
        BallPhysics->SetActiveFlight(false);
        OnWicket.Broadcast();
        return;
    }

    if (OtherComp->ComponentHasTag(TEXT("Pitch")) && Hit.ImpactNormal.Z > 0.45f)
    {
        BallPhysics->ResolvePitchBounce(Hit.ImpactNormal);
        if (bWasHitByBat)
        {
            bBouncedAfterBat = true;
        }
    }
}

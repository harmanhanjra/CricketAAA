#include "Ball/BallPhysicsComponent.h"
#include "Components/PrimitiveComponent.h"

UBallPhysicsComponent::UBallPhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UBallPhysicsComponent::AttachPhysicsBody(UPrimitiveComponent* InBody)
{
    PhysicsBody = InBody;
}

void UBallPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bActiveFlight || !PhysicsBody || !PhysicsBody->IsSimulatingPhysics())
    {
        return;
    }

    const FVector VelocityCm = PhysicsBody->GetPhysicsLinearVelocity();
    LastVelocity = VelocityCm;

    const FVector VelocityM = VelocityCm / 100.0f;
    const float SpeedM = VelocityM.Size();
    if (SpeedM < KINDA_SMALL_NUMBER)
    {
        return;
    }

    const FVector VelocityDir = VelocityM / SpeedM;
    const float DragN = 0.5f * AirDensityKgM3 * DragCoefficient * BallAreaM2 * SpeedM * SpeedM;
    const FVector DragForceN = -VelocityDir * DragN;

    const FVector MagnusDir = FVector::CrossProduct(SpinRadPerSec, VelocityM);
    const FVector MagnusForceN = MagnusDir * MagnusCoefficient;

    const FVector SwingAxis = FVector::CrossProduct(VelocityDir, FVector::UpVector).GetSafeNormal();
    const float SwingMagnitudeN = 0.00065f * SwingAmount * SpeedM * SpeedM;
    const FVector SwingForceN = SwingAxis * SwingMagnitudeN;

    const FVector TotalForceUE = (DragForceN + MagnusForceN + SwingForceN) * 100.0f;
    PhysicsBody->AddForce(TotalForceUE, NAME_None, false);
}

void UBallPhysicsComponent::ResolvePitchBounce(const FVector& ImpactNormal)
{
    if (!PhysicsBody)
    {
        return;
    }

    FVector Reflected = FVector::MirrorByVector(LastVelocity, ImpactNormal.GetSafeNormal());
    Reflected.Z *= PitchRestitution;
    Reflected.X *= PitchTangentialRetention;
    Reflected.Y *= PitchTangentialRetention;

    // Deterministic post-bounce deviation from spin; no arbitrary RNG.
    Reflected.Y += SpinRadPerSec.Z * 2.4f;
    Reflected.X += SpinRadPerSec.Y * 1.2f;

    PhysicsBody->SetPhysicsLinearVelocity(Reflected);
    SpinRadPerSec *= 0.86f;
}

#include "Batting/BattingComponent.h"
#include "Ball/CricketBall.h"
#include "Components/StaticMeshComponent.h"

UBattingComponent::UBattingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UBattingComponent::QueueShot(ACricketBall* InBall, const FCricketShotInput& ShotInput)
{
    if (!InBall || !InBall->IsBallLive())
    {
        return;
    }

    PendingBall = InBall;
    PendingShot = ShotInput;
    ShotWindowRemaining = 0.34f;
    TryResolveContact();
}

void UBattingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (ShotWindowRemaining <= 0.0f || !PendingBall)
    {
        return;
    }

    ShotWindowRemaining -= DeltaTime;
    TryResolveContact();

    if (ShotWindowRemaining <= 0.0f)
    {
        PendingBall = nullptr;
    }
}

void UBattingComponent::TryResolveContact()
{
    if (!PendingBall || !GetOwner())
    {
        return;
    }

    const FVector BatterLocation = GetOwner()->GetActorLocation();
    const FVector IdealContact = BatterLocation + FVector(-85.0f, PendingShot.Direction * 35.0f, 75.0f);
    const FVector BallLocation = PendingBall->GetActorLocation();
    const float Distance = FVector::Dist(BallLocation, IdealContact);

    if (Distance > ContactWindowCm)
    {
        return;
    }

    const float TimingQuality = FMath::Clamp(1.0f - Distance / ContactWindowCm, 0.0f, 1.0f);
    const FVector Incoming = PendingBall->GetBallMesh()->GetPhysicsLinearVelocity();
    const float IncomingSpeed = Incoming.Size();

    const float BaseSpeed = PendingShot.bLofted ? AggressiveShotSpeedCm : ControlledShotSpeedCm;
    const float ExitSpeed = FMath::Lerp(BaseSpeed * 0.45f, BaseSpeed, TimingQuality) + IncomingSpeed * 0.12f;

    // Batter faces toward -X. Lateral direction is continuous and deterministic.
    FVector Direction(-1.0f, FMath::Clamp(PendingShot.Direction, -1.0f, 1.0f) * 0.90f, PendingShot.bLofted ? 0.56f : 0.12f);
    Direction.Normalize();

    const float EdgeFactor = 1.0f - TimingQuality;
    Direction.Y += FMath::Clamp((BallLocation.Y - IdealContact.Y) / ContactWindowCm, -1.0f, 1.0f) * EdgeFactor * 0.65f;
    Direction.Normalize();

    const FVector NewSpin(
        0.0f,
        PendingShot.bLofted ? -18.0f : -7.0f,
        PendingShot.Direction * 30.0f * (0.3f + EdgeFactor)
    );

    PendingBall->ApplyBatContact(Direction * ExitSpeed * FMath::Clamp(PendingShot.Power, 0.35f, 1.0f), NewSpin);
    PendingBall = nullptr;
    ShotWindowRemaining = 0.0f;
}

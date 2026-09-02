#include "Bowling/BowlingComponent.h"
#include "Ball/CricketBall.h"

bool UBowlingComponent::DeliverBall(ACricketBall* Ball, const FBowlingInput& Input)
{
    if (!Ball || !GetOwner() || Ball->IsBallLive())
    {
        return false;
    }

    const FVector Start = GetOwner()->GetActorLocation() + ReleaseOffset;

    // Length maps from short (~+100 cm) to yorker (~+760 cm) along the pitch.
    const float BounceX = FMath::GetMappedRangeValueClamped(FVector2D(-1.0f, 1.0f), FVector2D(120.0f, 760.0f), Input.Length);
    const float BounceY = FMath::Clamp(Input.Line, -1.0f, 1.0f) * 135.0f;
    const FVector BounceTarget(BounceX, BounceY, 4.0f);

    const float HorizontalSpeed = FMath::Lerp(MinimumPaceCm, MaximumPaceCm, FMath::Clamp(Input.Pace01, 0.0f, 1.0f));
    const FVector DeltaXY(BounceTarget.X - Start.X, BounceTarget.Y - Start.Y, 0.0f);
    const float HorizontalDistance = FMath::Max(1.0f, DeltaXY.Size());
    const float FlightTime = HorizontalDistance / HorizontalSpeed;
    const FVector HorizontalVelocity = DeltaXY.GetSafeNormal() * HorizontalSpeed;

    constexpr float GravityCm = 980.0f;
    const float VerticalVelocity = (BounceTarget.Z - Start.Z + 0.5f * GravityCm * FlightTime * FlightTime) / FlightTime;
    const FVector InitialVelocity(HorizontalVelocity.X, HorizontalVelocity.Y, VerticalVelocity);

    const FVector SpinRadPerSec(0.0f, Input.Spin * 42.0f, Input.Spin * 76.0f);
    Ball->LaunchDelivery(Start, InitialVelocity, SpinRadPerSec, Input.Swing);
    return true;
}

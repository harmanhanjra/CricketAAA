#include "AI/BasicFieldingAIController.h"
#include "Ball/CricketBall.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"

ABasicFieldingAIController::ABasicFieldingAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ABasicFieldingAIController::CacheBall()
{
    if (Ball) return;

    for (TActorIterator<ACricketBall> It(GetWorld()); It; ++It)
    {
        Ball = *It;
        break;
    }
}

void ABasicFieldingAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    CacheBall();

    ACharacter* Fielder = Cast<ACharacter>(GetPawn());
    if (!Fielder || !Ball || !Ball->IsBallLive())
    {
        return;
    }

    FVector ToBall = Ball->GetActorLocation() - Fielder->GetActorLocation();
    ToBall.Z = 0.0f;
    const float Distance = ToBall.Size();

    if (Distance > 110.0f)
    {
        Fielder->AddMovementInput(ToBall.GetSafeNormal(), 1.0f);
        SetFocalPoint(Ball->GetActorLocation());
    }
    else
    {
        Ball->FieldBall();
    }
}

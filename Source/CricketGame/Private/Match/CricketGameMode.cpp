#include "Match/CricketGameMode.h"
#include "Core/CricketTypes.h"
#include "Match/CricketMatchState.h"
#include "Match/ScoreManager.h"
#include "Rules/MatchRulesManager.h"
#include "Ball/CricketBall.h"
#include "Characters/BatterCharacter.h"
#include "Characters/BowlerCharacter.h"
#include "Characters/FielderCharacter.h"
#include "AI/BasicFieldingAIController.h"
#include "World/CricketPracticePitch.h"
#include "Camera/CricketGameplayCamera.h"
#include "Player/CricketPlayerController.h"
#include "UI/ScoreHUD.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACricketGameMode::ACricketGameMode()
{
    GameStateClass = ACricketMatchState::StaticClass();
    PlayerControllerClass = ACricketPlayerController::StaticClass();
    HUDClass = AScoreHUD::StaticClass();
    DefaultPawnClass = nullptr;
}

void ACricketGameMode::BeginPlay()
{
    Super::BeginPlay();
    SpawnVerticalSliceWorld();

    if (ACricketMatchState* State = GetGameState<ACricketMatchState>())
    {
        State->InitializeMatch(DemoOvers);
    }
}

void ACricketGameMode::SpawnVerticalSliceWorld()
{
    if (!GetWorld()) return;

    GetWorld()->SpawnActor<ACricketPracticePitch>(ACricketPracticePitch::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);

    Bowler = GetWorld()->SpawnActor<ABowlerCharacter>(ABowlerCharacter::StaticClass(), FVector(-900.0f, 0.0f, 92.0f), FRotator(0.0f, 0.0f, 0.0f));
    Batter = GetWorld()->SpawnActor<ABatterCharacter>(ABatterCharacter::StaticClass(), FVector(900.0f, 0.0f, 92.0f), FRotator(0.0f, 180.0f, 0.0f));
    Ball = GetWorld()->SpawnActor<ACricketBall>(ACricketBall::StaticClass(), BallResetLocation, FRotator::ZeroRotator);
    GameplayCamera = GetWorld()->SpawnActor<ACricketGameplayCamera>(ACricketGameplayCamera::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);

    if (Ball)
    {
        Ball->OnBoundary.AddDynamic(this, &ACricketGameMode::HandleBoundary);
        Ball->OnWicket.AddDynamic(this, &ACricketGameMode::HandleWicket);
        Ball->OnBallDead.AddDynamic(this, &ACricketGameMode::HandleBallDead);
        Ball->ResetBall(BallResetLocation);
    }

    const FVector FielderLocations[] =
    {
        FVector(600.0f, 1050.0f, 92.0f),
        FVector(100.0f, -1500.0f, 92.0f),
        FVector(-1100.0f, 1250.0f, 92.0f)
    };

    for (const FVector& Location : FielderLocations)
    {
        AFielderCharacter* Fielder = GetWorld()->SpawnActor<AFielderCharacter>(AFielderCharacter::StaticClass(), Location, FRotator::ZeroRotator);
        ABasicFieldingAIController* Controller = GetWorld()->SpawnActor<ABasicFieldingAIController>();
        if (Fielder && Controller)
        {
            Controller->Possess(Fielder);
        }
    }
}

bool ACricketGameMode::BeginDelivery()
{
    ACricketMatchState* State = GetGameState<ACricketMatchState>();
    if (!State || !State->GetRulesManager() || State->GetPhase() != ECricketMatchPhase::AwaitingDelivery)
    {
        return false;
    }

    if (!State->GetRulesManager()->CanStartDelivery())
    {
        return false;
    }

    State->GetRulesManager()->BeginDelivery();
    return true;
}

void ACricketGameMode::RequestRun()
{
    ACricketMatchState* State = GetGameState<ACricketMatchState>();
    if (!State || State->GetPhase() != ECricketMatchPhase::BallLive || !State->GetRulesManager() || bRunInProgress || !Ball || !Ball->IsBallLive())
    {
        return;
    }

    bRunInProgress = true;
    GetWorldTimerManager().SetTimer(RunTimer, this, &ACricketGameMode::CompleteRequestedRun, 0.78f, false);
}

void ACricketGameMode::CompleteRequestedRun()
{
    bRunInProgress = false;
    ACricketMatchState* State = GetGameState<ACricketMatchState>();
    if (!State || State->GetPhase() != ECricketMatchPhase::BallLive || !State->GetRulesManager() || !Ball || !Ball->IsBallLive())
    {
        return;
    }

    State->GetRulesManager()->AddCompletedRun();
}

void ACricketGameMode::HandleBoundary(int32 Runs)
{
    FinishDelivery(Runs >= 6 ? ECricketDeliveryOutcome::BoundarySix : ECricketDeliveryOutcome::BoundaryFour);
}

void ACricketGameMode::HandleWicket()
{
    FinishDelivery(ECricketDeliveryOutcome::Wicket);
}

void ACricketGameMode::HandleBallDead()
{
    FinishDelivery(ECricketDeliveryOutcome::Runs);
}

void ACricketGameMode::FinishDelivery(ECricketDeliveryOutcome Outcome, int32 EventRuns)
{
    ACricketMatchState* State = GetGameState<ACricketMatchState>();
    if (!State || !State->GetRulesManager() || !State->GetRulesManager()->IsDeliveryActive())
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(RunTimer);
    bRunInProgress = false;
    State->GetRulesManager()->FinalizeDelivery(Outcome, EventRuns, true);
    State->SetPhase(ECricketMatchPhase::DeliveryComplete);

    if (State->GetScoreManager() && State->GetScoreManager()->IsInningsComplete())
    {
        State->SetPhase(ECricketMatchPhase::MatchComplete);
        return;
    }

    GetWorldTimerManager().SetTimer(ResetTimer, this, &ACricketGameMode::ResetForNextDelivery, 0.65f, false);
}

void ACricketGameMode::ResetForNextDelivery()
{
    if (Ball)
    {
        Ball->ResetBall(BallResetLocation);
    }

    if (ACricketMatchState* State = GetGameState<ACricketMatchState>())
    {
        State->SetPhase(ECricketMatchPhase::AwaitingDelivery);
    }
}

void ACricketGameMode::ForceResetDelivery()
{
    GetWorldTimerManager().ClearTimer(ResetTimer);
    GetWorldTimerManager().ClearTimer(RunTimer);
    bRunInProgress = false;

    if (Ball)
    {
        Ball->ResetBall(BallResetLocation);
    }

    if (ACricketMatchState* State = GetGameState<ACricketMatchState>())
    {
        if (State->GetRulesManager() && State->GetRulesManager()->IsDeliveryActive())
        {
            State->GetRulesManager()->FinalizeDelivery(ECricketDeliveryOutcome::DotBall, 0, true);
        }

        if (State->GetScoreManager() && State->GetScoreManager()->IsInningsComplete())
        {
            State->SetPhase(ECricketMatchPhase::MatchComplete);
        }
        else
        {
            State->SetPhase(ECricketMatchPhase::AwaitingDelivery);
        }
    }
}

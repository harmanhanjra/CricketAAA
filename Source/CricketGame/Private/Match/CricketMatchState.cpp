#include "Match/CricketMatchState.h"
#include "Match/ScoreManager.h"
#include "Rules/MatchRulesManager.h"

void ACricketMatchState::BeginPlay()
{
    Super::BeginPlay();

    if (!ScoreManager)
    {
        ScoreManager = NewObject<UScoreManager>(this, TEXT("ScoreManager"));
    }
    if (!RulesManager)
    {
        RulesManager = NewObject<UMatchRulesManager>(this, TEXT("RulesManager"));
    }
}

void ACricketMatchState::InitializeMatch(int32 MaxOvers)
{
    if (!ScoreManager)
    {
        ScoreManager = NewObject<UScoreManager>(this, TEXT("ScoreManager"));
    }
    if (!RulesManager)
    {
        RulesManager = NewObject<UMatchRulesManager>(this, TEXT("RulesManager"));
    }

    ScoreManager->Reset(MaxOvers);
    RulesManager->Initialize(ScoreManager);
    MatchPhase = ECricketMatchPhase::AwaitingDelivery;
}

#include "Rules/MatchRulesManager.h"
#include "Match/ScoreManager.h"

void UMatchRulesManager::Initialize(UScoreManager* InScoreManager)
{
    ScoreManager = InScoreManager;
    RunsThisDelivery = 0;
    bDeliveryActive = false;
}

bool UMatchRulesManager::CanStartDelivery() const
{
    return ScoreManager && !ScoreManager->IsInningsComplete() && !bDeliveryActive;
}

void UMatchRulesManager::BeginDelivery()
{
    if (CanStartDelivery())
    {
        RunsThisDelivery = 0;
        bDeliveryActive = true;
    }
}

void UMatchRulesManager::AddCompletedRun()
{
    if (bDeliveryActive && ScoreManager)
    {
        ++RunsThisDelivery;
        ScoreManager->AddRuns(1);
    }
}

void UMatchRulesManager::FinalizeDelivery(ECricketDeliveryOutcome Outcome, int32 EventRuns, bool bLegalBall)
{
    if (!bDeliveryActive || !ScoreManager)
    {
        return;
    }

    switch (Outcome)
    {
        case ECricketDeliveryOutcome::BoundaryFour:
            ScoreManager->AddRuns(4);
            break;
        case ECricketDeliveryOutcome::BoundarySix:
            ScoreManager->AddRuns(6);
            break;
        case ECricketDeliveryOutcome::Wicket:
            ScoreManager->AddWicket();
            break;
        case ECricketDeliveryOutcome::Wide:
        case ECricketDeliveryOutcome::NoBall:
            ScoreManager->AddRuns(FMath::Max(1, EventRuns));
            bLegalBall = false;
            break;
        case ECricketDeliveryOutcome::Runs:
            if (EventRuns > RunsThisDelivery)
            {
                ScoreManager->AddRuns(EventRuns - RunsThisDelivery);
            }
            break;
        case ECricketDeliveryOutcome::DotBall:
        default:
            break;
    }

    if (bLegalBall)
    {
        ScoreManager->RecordLegalBall();
    }

    bDeliveryActive = false;
}

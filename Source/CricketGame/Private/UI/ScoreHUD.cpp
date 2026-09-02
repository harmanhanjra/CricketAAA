#include "UI/ScoreHUD.h"
#include "Match/CricketMatchState.h"
#include "Match/ScoreManager.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void AScoreHUD::DrawHUD()
{
    Super::DrawHUD();

    const ACricketMatchState* State = GetWorld() ? GetWorld()->GetGameState<ACricketMatchState>() : nullptr;
    if (!State || !State->GetScoreManager() || !Canvas || !GEngine)
    {
        return;
    }

    const UScoreManager* Score = State->GetScoreManager();
    const FString ScoreLine = FString::Printf(TEXT("CRICKET VERTICAL SLICE    %d/%d    Overs %s"), Score->Runs, Score->Wickets, *Score->GetOversText());
    DrawText(ScoreLine, FLinearColor::White, 40.0f, 35.0f, GEngine->GetLargeFont(), 1.0f, false);

    FString PhaseText = TEXT("SETUP");
    switch (State->GetPhase())
    {
        case ECricketMatchPhase::AwaitingDelivery: PhaseText = TEXT("READY TO BOWL"); break;
        case ECricketMatchPhase::BallLive: PhaseText = TEXT("BALL LIVE"); break;
        case ECricketMatchPhase::DeliveryComplete: PhaseText = TEXT("DELIVERY COMPLETE"); break;
        case ECricketMatchPhase::MatchComplete: PhaseText = TEXT("MATCH COMPLETE"); break;
        default: break;
    }

    DrawText(PhaseText, FLinearColor(0.85f, 0.90f, 1.0f), 42.0f, 82.0f, GEngine->GetMediumFont(), 1.0f, false);
    DrawText(TEXT("SPACE bowl | Mouse aim | LMB controlled shot | RMB aggressive/lofted | R run | TAB camera | BACKSPACE reset"),
        FLinearColor::White, 40.0f, Canvas->ClipY - 62.0f, GEngine->GetSmallFont(), 1.0f, false);
}

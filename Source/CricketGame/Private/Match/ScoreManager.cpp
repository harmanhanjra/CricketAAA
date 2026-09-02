#include "Match/ScoreManager.h"

void UScoreManager::Reset(int32 InMaxOvers)
{
    Runs = 0;
    Wickets = 0;
    LegalBalls = 0;
    MaxOvers = FMath::Max(1, InMaxOvers);
}

void UScoreManager::AddRuns(int32 Amount)
{
    Runs += FMath::Max(0, Amount);
}

void UScoreManager::AddWicket()
{
    Wickets = FMath::Clamp(Wickets + 1, 0, 10);
}

void UScoreManager::RecordLegalBall()
{
    ++LegalBalls;
}

FString UScoreManager::GetOversText() const
{
    return FString::Printf(TEXT("%d.%d"), LegalBalls / 6, LegalBalls % 6);
}

bool UScoreManager::IsInningsComplete() const
{
    return Wickets >= 10 || LegalBalls >= MaxOvers * 6;
}

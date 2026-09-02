#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Core/CricketTypes.h"
#include "MatchRulesManager.generated.h"

class UScoreManager;

UCLASS(BlueprintType)
class CRICKETGAME_API UMatchRulesManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UScoreManager* InScoreManager);

    UFUNCTION(BlueprintCallable)
    bool CanStartDelivery() const;

    UFUNCTION(BlueprintCallable)
    void BeginDelivery();

    UFUNCTION(BlueprintCallable)
    void AddCompletedRun();

    UFUNCTION(BlueprintCallable)
    void FinalizeDelivery(ECricketDeliveryOutcome Outcome, int32 EventRuns = 0, bool bLegalBall = true);

    UFUNCTION(BlueprintPure)
    int32 GetRunsThisDelivery() const { return RunsThisDelivery; }

    UFUNCTION(BlueprintPure)
    bool IsDeliveryActive() const { return bDeliveryActive; }

private:
    UPROPERTY()
    TObjectPtr<UScoreManager> ScoreManager;

    int32 RunsThisDelivery = 0;
    bool bDeliveryActive = false;
};

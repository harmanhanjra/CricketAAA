#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/CricketTypes.h"
#include "CricketMatchState.generated.h"

class UScoreManager;
class UMatchRulesManager;

UCLASS()
class CRICKETGAME_API ACricketMatchState : public AGameStateBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable)
    void InitializeMatch(int32 MaxOvers);

    UFUNCTION(BlueprintCallable)
    void SetPhase(ECricketMatchPhase NewPhase) { MatchPhase = NewPhase; }

    UFUNCTION(BlueprintPure)
    ECricketMatchPhase GetPhase() const { return MatchPhase; }

    UFUNCTION(BlueprintPure)
    UScoreManager* GetScoreManager() const { return ScoreManager; }

    UFUNCTION(BlueprintPure)
    UMatchRulesManager* GetRulesManager() const { return RulesManager; }

private:
    UPROPERTY()
    TObjectPtr<UScoreManager> ScoreManager;

    UPROPERTY()
    TObjectPtr<UMatchRulesManager> RulesManager;

    UPROPERTY()
    ECricketMatchPhase MatchPhase = ECricketMatchPhase::Setup;
};

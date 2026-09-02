#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ScoreManager.generated.h"

UCLASS(BlueprintType)
class CRICKETGAME_API UScoreManager : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void Reset(int32 InMaxOvers = 2);

    UFUNCTION(BlueprintCallable)
    void AddRuns(int32 Amount);

    UFUNCTION(BlueprintCallable)
    void AddWicket();

    UFUNCTION(BlueprintCallable)
    void RecordLegalBall();

    UFUNCTION(BlueprintPure)
    FString GetOversText() const;

    UFUNCTION(BlueprintPure)
    bool IsInningsComplete() const;

    UPROPERTY(BlueprintReadOnly)
    int32 Runs = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 Wickets = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 LegalBalls = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 MaxOvers = 2;
};

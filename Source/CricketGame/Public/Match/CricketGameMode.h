#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/CricketTypes.h"
#include "CricketGameMode.generated.h"

class ACricketBall;
class ABatterCharacter;
class ABowlerCharacter;
class ACricketGameplayCamera;

UCLASS()
class CRICKETGAME_API ACricketGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACricketGameMode();
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable)
    bool BeginDelivery();

    UFUNCTION(BlueprintCallable)
    void RequestRun();

    UFUNCTION(BlueprintCallable)
    void ForceResetDelivery();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Match")
    int32 DemoOvers = 2;

private:
    UFUNCTION()
    void HandleBoundary(int32 Runs);

    UFUNCTION()
    void HandleWicket();

    UFUNCTION()
    void HandleBallDead();

    void FinishDelivery(ECricketDeliveryOutcome Outcome, int32 EventRuns = 0);
    void ResetForNextDelivery();
    void CompleteRequestedRun();
    void SpawnVerticalSliceWorld();

    UPROPERTY()
    TObjectPtr<ACricketBall> Ball;

    UPROPERTY()
    TObjectPtr<ABatterCharacter> Batter;

    UPROPERTY()
    TObjectPtr<ABowlerCharacter> Bowler;

    UPROPERTY()
    TObjectPtr<ACricketGameplayCamera> GameplayCamera;

    FVector BallResetLocation = FVector(-780.0f, 0.0f, 220.0f);
    FTimerHandle ResetTimer;
    FTimerHandle RunTimer;
    bool bRunInProgress = false;
};

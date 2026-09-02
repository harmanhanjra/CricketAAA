#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/CricketTypes.h"
#include "BattingComponent.generated.h"

class ACricketBall;

UCLASS(ClassGroup=(Cricket), meta=(BlueprintSpawnableComponent))
class CRICKETGAME_API UBattingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBattingComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable)
    void QueueShot(ACricketBall* InBall, const FCricketShotInput& ShotInput);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Batting")
    float ContactWindowCm = 185.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Batting")
    float ControlledShotSpeedCm = 2400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Batting")
    float AggressiveShotSpeedCm = 3900.0f;

private:
    void TryResolveContact();

    UPROPERTY()
    TObjectPtr<ACricketBall> PendingBall;

    FCricketShotInput PendingShot;
    float ShotWindowRemaining = 0.0f;
};

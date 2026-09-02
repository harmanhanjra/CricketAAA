#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/CricketTypes.h"
#include "CricketPlayerController.generated.h"

class ACricketBall;
class ABatterCharacter;
class ABowlerCharacter;
class ACricketGameplayCamera;

UCLASS()
class CRICKETGAME_API ACricketPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ACricketPlayerController();
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    void CacheActors();
    void Bowl();
    void ControlledShot();
    void AggressiveShot();
    void RequestRun();
    void ResetDelivery();
    void SwitchCamera();
    void AimHorizontal(float Value);
    void AimVertical(float Value);
    void PlayShot(bool bAggressive);

    UPROPERTY()
    TObjectPtr<ACricketBall> Ball;

    UPROPERTY()
    TObjectPtr<ABatterCharacter> Batter;

    UPROPERTY()
    TObjectPtr<ABowlerCharacter> Bowler;

    UPROPERTY()
    TObjectPtr<ACricketGameplayCamera> GameplayCamera;

    float AimLine = 0.0f;
    float AimLength = 0.15f;
    int32 CameraIndex = 0;
};

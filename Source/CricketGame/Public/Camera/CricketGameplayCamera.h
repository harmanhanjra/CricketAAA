#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/CricketTypes.h"
#include "CricketGameplayCamera.generated.h"

class UCameraComponent;

UCLASS()
class CRICKETGAME_API ACricketGameplayCamera : public AActor
{
    GENERATED_BODY()

public:
    ACricketGameplayCamera();

    UFUNCTION(BlueprintCallable)
    void SetCameraMode(ECricketCameraMode NewMode);

    UFUNCTION(BlueprintPure)
    ECricketCameraMode GetCameraMode() const { return CameraMode; }

private:
    void ApplyCameraMode();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UCameraComponent> Camera;

    ECricketCameraMode CameraMode = ECricketCameraMode::Broadcast;
};

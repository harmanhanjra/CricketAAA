#include "Camera/CricketGameplayCamera.h"
#include "Camera/CameraComponent.h"

ACricketGameplayCamera::ACricketGameplayCamera()
{
    PrimaryActorTick.bCanEverTick = false;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    SetRootComponent(Camera);
    Camera->SetFieldOfView(48.0f);
    ApplyCameraMode();
}

void ACricketGameplayCamera::SetCameraMode(ECricketCameraMode NewMode)
{
    CameraMode = NewMode;
    ApplyCameraMode();
}

void ACricketGameplayCamera::ApplyCameraMode()
{
    FVector Location;
    FVector Target;

    switch (CameraMode)
    {
        case ECricketCameraMode::BehindBowler:
            Location = FVector(-1800.0f, -220.0f, 480.0f);
            Target = FVector(650.0f, 0.0f, 80.0f);
            break;
        case ECricketCameraMode::BehindBatter:
            Location = FVector(1450.0f, -120.0f, 330.0f);
            Target = FVector(-300.0f, 0.0f, 80.0f);
            break;
        case ECricketCameraMode::Broadcast:
        default:
            Location = FVector(1400.0f, -2600.0f, 1450.0f);
            Target = FVector(0.0f, 0.0f, 60.0f);
            break;
    }

    SetActorLocation(Location);
    SetActorRotation((Target - Location).Rotation());
}

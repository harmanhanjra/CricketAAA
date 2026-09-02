#include "Player/CricketPlayerController.h"
#include "Ball/CricketBall.h"
#include "Characters/BatterCharacter.h"
#include "Characters/BowlerCharacter.h"
#include "Batting/BattingComponent.h"
#include "Bowling/BowlingComponent.h"
#include "Camera/CricketGameplayCamera.h"
#include "Match/CricketGameMode.h"
#include "Match/CricketMatchState.h"
#include "EngineUtils.h"
#include "Engine/World.h"

ACricketPlayerController::ACricketPlayerController()
{
    bShowMouseCursor = false;
}

void ACricketPlayerController::BeginPlay()
{
    Super::BeginPlay();
    CacheActors();
    if (GameplayCamera)
    {
        SetViewTarget(GameplayCamera);
    }
}

void ACricketPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    check(InputComponent);

    InputComponent->BindAction(TEXT("Bowl"), IE_Pressed, this, &ACricketPlayerController::Bowl);
    InputComponent->BindAction(TEXT("BatControlled"), IE_Pressed, this, &ACricketPlayerController::ControlledShot);
    InputComponent->BindAction(TEXT("BatAggressive"), IE_Pressed, this, &ACricketPlayerController::AggressiveShot);
    InputComponent->BindAction(TEXT("Run"), IE_Pressed, this, &ACricketPlayerController::RequestRun);
    InputComponent->BindAction(TEXT("ResetDelivery"), IE_Pressed, this, &ACricketPlayerController::ResetDelivery);
    InputComponent->BindAction(TEXT("SwitchCamera"), IE_Pressed, this, &ACricketPlayerController::SwitchCamera);
    InputComponent->BindAxis(TEXT("AimHorizontal"), this, &ACricketPlayerController::AimHorizontal);
    InputComponent->BindAxis(TEXT("AimVertical"), this, &ACricketPlayerController::AimVertical);
}

void ACricketPlayerController::CacheActors()
{
    if (!GetWorld()) return;

    for (TActorIterator<ACricketBall> It(GetWorld()); It && !Ball; ++It) Ball = *It;
    for (TActorIterator<ABatterCharacter> It(GetWorld()); It && !Batter; ++It) Batter = *It;
    for (TActorIterator<ABowlerCharacter> It(GetWorld()); It && !Bowler; ++It) Bowler = *It;
    for (TActorIterator<ACricketGameplayCamera> It(GetWorld()); It && !GameplayCamera; ++It) GameplayCamera = *It;
}

void ACricketPlayerController::AimHorizontal(float Value)
{
    AimLine = FMath::Clamp(AimLine + Value * 0.015f, -1.0f, 1.0f);
}

void ACricketPlayerController::AimVertical(float Value)
{
    AimLength = FMath::Clamp(AimLength + Value * 0.015f, -1.0f, 1.0f);
}

void ACricketPlayerController::Bowl()
{
    CacheActors();
    ACricketMatchState* State = GetWorld() ? GetWorld()->GetGameState<ACricketMatchState>() : nullptr;
    if (!State || State->GetPhase() != ECricketMatchPhase::AwaitingDelivery || !Bowler || !Ball)
    {
        return;
    }

    FBowlingInput Input;
    Input.Line = AimLine;
    Input.Length = AimLength;
    Input.Pace01 = 0.82f;
    Input.Swing = FMath::Clamp(AimLine * 0.35f, -0.35f, 0.35f);
    Input.Spin = FMath::Clamp(AimLine * 0.15f, -0.15f, 0.15f);

    ACricketGameMode* GM = GetWorld()->GetAuthGameMode<ACricketGameMode>();
    if (GM && GM->BeginDelivery() && Bowler->GetBowlingComponent()->DeliverBall(Ball, Input))
    {
        State->SetPhase(ECricketMatchPhase::BallLive);
    }
}

void ACricketPlayerController::ControlledShot()
{
    PlayShot(false);
}

void ACricketPlayerController::AggressiveShot()
{
    PlayShot(true);
}

void ACricketPlayerController::PlayShot(bool bAggressive)
{
    CacheActors();
    ACricketMatchState* State = GetWorld() ? GetWorld()->GetGameState<ACricketMatchState>() : nullptr;
    if (!State || State->GetPhase() != ECricketMatchPhase::BallLive || !Batter || !Ball)
    {
        return;
    }

    FCricketShotInput Shot;
    Shot.Direction = AimLine;
    Shot.Power = bAggressive ? 1.0f : 0.74f;
    Shot.bLofted = bAggressive;
    Batter->GetBattingComponent()->QueueShot(Ball, Shot);
}

void ACricketPlayerController::RequestRun()
{
    if (ACricketGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ACricketGameMode>() : nullptr)
    {
        GM->RequestRun();
    }
}

void ACricketPlayerController::ResetDelivery()
{
    if (ACricketGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ACricketGameMode>() : nullptr)
    {
        GM->ForceResetDelivery();
    }
}

void ACricketPlayerController::SwitchCamera()
{
    CacheActors();
    if (!GameplayCamera) return;

    CameraIndex = (CameraIndex + 1) % 3;
    GameplayCamera->SetCameraMode(static_cast<ECricketCameraMode>(CameraIndex));
    SetViewTargetWithBlend(GameplayCamera, 0.18f);
}

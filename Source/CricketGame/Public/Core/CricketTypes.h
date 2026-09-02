#pragma once

#include "CoreMinimal.h"
#include "CricketTypes.generated.h"

UENUM(BlueprintType)
enum class ECricketMatchPhase : uint8
{
    Setup,
    AwaitingDelivery,
    BallLive,
    DeliveryComplete,
    MatchComplete
};

UENUM(BlueprintType)
enum class ECricketDeliveryOutcome : uint8
{
    DotBall,
    Runs,
    BoundaryFour,
    BoundarySix,
    Wicket,
    Wide,
    NoBall
};

UENUM(BlueprintType)
enum class ECricketCameraMode : uint8
{
    Broadcast,
    BehindBowler,
    BehindBatter
};

USTRUCT(BlueprintType)
struct FCricketShotInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Power = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLofted = false;
};

USTRUCT(BlueprintType)
struct FBowlingInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Line = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Length = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pace01 = 0.78f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Swing = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Spin = 0.0f;
};

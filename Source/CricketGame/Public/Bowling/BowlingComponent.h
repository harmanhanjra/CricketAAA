#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/CricketTypes.h"
#include "BowlingComponent.generated.h"

class ACricketBall;

UCLASS(ClassGroup=(Cricket), meta=(BlueprintSpawnableComponent))
class CRICKETGAME_API UBowlingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    bool DeliverBall(ACricketBall* Ball, const FBowlingInput& Input);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bowling")
    float MinimumPaceCm = 2200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bowling")
    float MaximumPaceCm = 4050.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bowling")
    FVector ReleaseOffset = FVector(70.0f, 0.0f, 210.0f);
};

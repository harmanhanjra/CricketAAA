#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BasicFieldingAIController.generated.h"

class ACricketBall;

UCLASS()
class CRICKETGAME_API ABasicFieldingAIController : public AAIController
{
    GENERATED_BODY()

public:
    ABasicFieldingAIController();
    virtual void Tick(float DeltaSeconds) override;

private:
    void CacheBall();

    UPROPERTY()
    TObjectPtr<ACricketBall> Ball;
};

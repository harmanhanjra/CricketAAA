#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BowlerCharacter.generated.h"

class UBowlingComponent;
class UStaticMeshComponent;

UCLASS()
class CRICKETGAME_API ABowlerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABowlerCharacter();

    UFUNCTION(BlueprintPure)
    UBowlingComponent* GetBowlingComponent() const { return BowlingComponent; }

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBowlingComponent> BowlingComponent;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> BodyVisual;
};

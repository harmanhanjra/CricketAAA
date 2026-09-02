#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BatterCharacter.generated.h"

class UBattingComponent;
class UStaticMeshComponent;

UCLASS()
class CRICKETGAME_API ABatterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABatterCharacter();

    UFUNCTION(BlueprintPure)
    UBattingComponent* GetBattingComponent() const { return BattingComponent; }

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBattingComponent> BattingComponent;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> BodyVisual;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> BatVisual;
};

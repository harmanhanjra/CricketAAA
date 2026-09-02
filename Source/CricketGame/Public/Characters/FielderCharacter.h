#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FielderCharacter.generated.h"

class UStaticMeshComponent;

UCLASS()
class CRICKETGAME_API AFielderCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AFielderCharacter();

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> BodyVisual;
};

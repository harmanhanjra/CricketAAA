#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CricketPracticePitch.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UStaticMesh;

UCLASS()
class CRICKETGAME_API ACricketPracticePitch : public AActor
{
    GENERATED_BODY()

public:
    ACricketPracticePitch();

private:
    UStaticMeshComponent* CreateMeshPart(const FName Name, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale, const FName Tag);

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;
};

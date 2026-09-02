#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallPhysicsComponent.generated.h"

class UPrimitiveComponent;

UCLASS(ClassGroup=(Cricket), meta=(BlueprintSpawnableComponent))
class CRICKETGAME_API UBallPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBallPhysicsComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void AttachPhysicsBody(UPrimitiveComponent* InBody);
    void SetSpinRadPerSec(const FVector& InSpin) { SpinRadPerSec = InSpin; }
    void SetSwingAmount(float InSwing) { SwingAmount = FMath::Clamp(InSwing, -1.0f, 1.0f); }
    void SetActiveFlight(bool bInActive) { bActiveFlight = bInActive; }
    void ResolvePitchBounce(const FVector& ImpactNormal);

    UFUNCTION(BlueprintPure)
    FVector GetLastVelocity() const { return LastVelocity; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cricket Physics")
    float DragCoefficient = 0.32f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cricket Physics")
    float AirDensityKgM3 = 1.225f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cricket Physics")
    float BallAreaM2 = 0.0042f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cricket Physics")
    float MagnusCoefficient = 0.00042f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pitch")
    float PitchRestitution = 0.56f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pitch")
    float PitchTangentialRetention = 0.82f;

private:
    UPROPERTY()
    TObjectPtr<UPrimitiveComponent> PhysicsBody;

    FVector SpinRadPerSec = FVector::ZeroVector;
    FVector LastVelocity = FVector::ZeroVector;
    float SwingAmount = 0.0f;
    bool bActiveFlight = false;
};

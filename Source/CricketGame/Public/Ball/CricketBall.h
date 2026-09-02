#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CricketBall.generated.h"

class UStaticMeshComponent;
class UPrimitiveComponent;
class UBallPhysicsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCricketBoundary, int32, Runs);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCricketWicket);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCricketBallDead);

UCLASS()
class CRICKETGAME_API ACricketBall : public AActor
{
    GENERATED_BODY()

public:
    ACricketBall();
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable)
    void LaunchDelivery(const FVector& StartLocation, const FVector& InitialVelocityCm, const FVector& SpinRadPerSec, float SwingAmount);

    UFUNCTION(BlueprintCallable)
    void ApplyBatContact(const FVector& ExitVelocityCm, const FVector& NewSpinRadPerSec);

    UFUNCTION(BlueprintCallable)
    void ResetBall(const FVector& NewLocation);

    UFUNCTION(BlueprintCallable)
    void FieldBall();

    UFUNCTION(BlueprintPure)
    bool IsBallLive() const { return bBallLive; }

    UFUNCTION(BlueprintPure)
    UStaticMeshComponent* GetBallMesh() const { return BallMesh; }

    UPROPERTY(BlueprintAssignable)
    FOnCricketBoundary OnBoundary;

    UPROPERTY(BlueprintAssignable)
    FOnCricketWicket OnWicket;

    UPROPERTY(BlueprintAssignable)
    FOnCricketBallDead OnBallDead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cricket")
    float BoundaryRadiusCm = 6500.0f;

private:
    UFUNCTION()
    void HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> BallMesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBallPhysicsComponent> BallPhysics;

    bool bBallLive = false;
    bool bWasHitByBat = false;
    bool bBouncedAfterBat = false;
    bool bBoundaryReported = false;
    float LiveSeconds = 0.0f;
    float SlowSeconds = 0.0f;
};

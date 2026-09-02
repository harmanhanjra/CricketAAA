#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ScoreHUD.generated.h"

UCLASS()
class CRICKETGAME_API AScoreHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;
};

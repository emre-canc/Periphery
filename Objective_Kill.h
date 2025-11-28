#pragma once
#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "Objective_Count.generated.h"

UCLASS(DisplayName = "Kill Target")
class INSIDETFV03_API UObjective_Kill : public UMissionObjective
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Rules")
    FGameplayTag EnemyDeathTag;

    UPROPERTY(EditAnywhere, Category = "Rules")
    int32 RequiredKills = 1;

    UPROPERTY(EditAnywhere, Category = "Rules")
    bool bRequirePlayerSource = true;
};
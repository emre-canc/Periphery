#pragma once
#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "Objective_Kill.generated.h"

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

    virtual bool OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const override;
    
    virtual bool IsComplete(const FObjectiveRuntimeState& RuntimeState) const override;

};
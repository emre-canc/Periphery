#pragma once
#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "Objective_Checklist.generated.h"

UCLASS(DisplayName = "Checklist (Complete All)")
class INSIDETFV03_API UObjective_Checklist : public UMissionObjective
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Rules")
    TArray<FGameplayTag> RequiredTags;

};
#pragma once
#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "Objective_Checklist.generated.h"

UCLASS(DisplayName = "Checklist (Complete All)")
class INSIDETFV03_API UObjective_Checklist : public UMissionObjective
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "3. Rules")
    TArray<FGameplayTag> RequiredTags;

    virtual bool OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const override;
    
    virtual bool IsComplete(const FObjectiveRuntimeState& RuntimeState) const override;
};
// Periphery -- EvEGames -- Objective_Simple.h
#pragma once
#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "Objective_Simple.generated.h"

UCLASS(DisplayName = "Simple (Do Once)")
class INSIDETFV03_API UObjective_Simple : public UMissionObjective
{
    GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, Category = "Rules")
    FGameplayTag TargetEvent;

    UPROPERTY(EditAnywhere, Category = "Rules")
    bool bCountPastEvents = false;


    virtual void InitializeRuntime(FObjectiveRuntimeState& RuntimeState) const override;
    
    virtual bool OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const override;
    
    virtual bool IsComplete(const FObjectiveRuntimeState& RuntimeState) const override;
};
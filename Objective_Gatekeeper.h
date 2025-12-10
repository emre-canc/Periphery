#pragma once
#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "Objective_Gatekeeper.generated.h"

UCLASS(DisplayName = "Gatekeeper (Wait for Objectives)")
class INSIDETFV03_API UObjective_Gatekeeper : public UMissionObjective
{
    GENERATED_BODY()

public:
    // The list of Objective IDs the gatekeeper is waiting for.
    // Select the Objectives (e.g., "CleanBathroom", "CleanSales").
    UPROPERTY(EditAnywhere, Category = "Rules")
    TArray<FGameplayTag> RequiredObjectives;

    // --- 1. Ignore Standard Events ---
    // Returns false because this objective doesn't care about kills, locations, or items.
    virtual bool OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, 
        AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const override { return false; }

    // --- 2. Listen for Objective Completion ---
    // This is called by the Subsystem whenever ANY objective finishes.
    virtual bool OnObjectiveCompleted(const FGameplayTag& CompletedObjectiveID, FObjectiveRuntimeState& RuntimeState) const override;


    // --- 3. Check for Full Completion ---
    virtual bool IsComplete(const FObjectiveRuntimeState& RuntimeState) const override;

};
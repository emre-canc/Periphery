#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "MissionStructs.h"
#include "MissionObjective.generated.h"

/**
 * Base class for all Mission Objectives.
 * "EditInlineNew" allows you to create this object directly inside the MissionData asset.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class INSIDETFV03_API UMissionObjective : public UObject
{
    GENERATED_BODY()

public:
    // --- Objective Info ---

    // The tag used to look up this objective in the Subsystem
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1. Identity")
    FGameplayTag ObjectiveID;

    // A brief summary of what the objective entails.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1. Identity")
    FText Description;


    // ---Objective Start ---

    // All objectives with this enabled will automatically start
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "2. Activation")
    bool bStartAutomatically = false;

    //Send Command, Update Environment, Toggle Widget, Toggle Sequence
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "2. Activation")
    TArray<FObjectiveActionDefinition> StartActions;

    // --- Objective Steps  ---
    //Implemented in child classes
 

    // --- Objective Completion ---

    //Send Command, Update Environment, Toggle Widget, Toggle Sequence
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "4. Outcome")
    TArray<FObjectiveActionDefinition> CompleteActions;

    // Which objectives activate when this one is finished?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "4. Outcome")
    TArray<FGameplayTag> NextObjectiveIDs;

    // --- Virtual API ---
    virtual bool IsComplete() const { return false; }     // Child classes can override this for custom code-based completion checks

    virtual bool OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, 
        AActor* SourceActor, FObjectiveRuntimeState& ObjectiveRuntime) const { return false; }
	
};
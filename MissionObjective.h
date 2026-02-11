// Periphery -- EvEGames

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Missions/MissionStructs.h"
#include "Missions/Actions/MissionAction.h"
#include "MissionObjective.generated.h"

/**
 * Base class for all Mission Objectives.
 * "EditInlineNew" allows you to create this object directly inside the MissionData asset.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced, meta=(PrioritizeCategories="Identity Activation Rules Outcome"))
class INSIDETFV03_API UMissionObjective : public UObject
{
    GENERATED_BODY()

public:
    // --- Objective Info ---

    // The name that will be used in the editor.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText Name;

    // A brief summary of what the objective entails.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText Description;

    // The tag used to look up this objective in the Subsystem.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FGameplayTag ObjectiveID;


    // ---Objective Start ---

    // The first value in ObjectiveArray will always automatically start.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Activation")
    bool bStartAutomatically = false;

    UPROPERTY(EditAnywhere, Instanced, Category = "Activation")
    TArray<TObjectPtr<UMissionAction>> StartActions;

    // --- Objective Rules  ---
    //Implemented in child classes

 
    // --- Objective Completion ---

    UPROPERTY(EditAnywhere, Instanced, Category = "Outcome")
    TArray<TObjectPtr<UMissionAction>> CompleteActions;

    // Which objectives activate when this one is finished?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Outcome")
    TArray<FGameplayTag> NextObjectiveIDs;



    // --- Virtual API ---

    virtual void InitializeRuntime(FObjectiveRuntimeState& RuntimeState) const 
        { RuntimeState.ObjectiveState = EProgressState::InProgress; }

    virtual bool OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, 
        AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const { return false; }

    /** Called when ANY other objective in the mission is completed. */
    virtual bool OnObjectiveCompleted(const FGameplayTag& CompletedObjectiveID,
        FObjectiveRuntimeState& RuntimeState) const {    return false;  } 

    virtual bool IsComplete(const FObjectiveRuntimeState& RuntimeState) const { return false; }     // Child classes can override this for custom code-based completion checks

	
};
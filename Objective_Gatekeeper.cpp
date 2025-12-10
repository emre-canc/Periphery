

#include "Missions/Objectives/Objective_Gatekeeper.h"

bool UObjective_Gatekeeper::OnObjectiveCompleted(const FGameplayTag& CompletedObjectiveID, FObjectiveRuntimeState& RuntimeState) const
{
    // Is the finished objective one of the ones we are waiting for?
    if (RequiredObjectives.Contains(CompletedObjectiveID))
    {
        FName Key = CompletedObjectiveID.GetTagName();
        
        // Mark it as done in our storage if it isn't already
        if (!RuntimeState.BoolStorage.FindRef(Key))
        {
            RuntimeState.BoolStorage.Add(Key, true);
            return true; // State changed! This prompts the Subsystem to check IsComplete().
        }
    }
    return false;
}

bool UObjective_Gatekeeper::IsComplete(const FObjectiveRuntimeState& RuntimeState) const
{
    // Iterate through our required list.
    // If ANY requirement is missing from storage, we are not done.
    for (const FGameplayTag& Tag : RequiredObjectives)
    {
        if (!RuntimeState.BoolStorage.FindRef(Tag.GetTagName()))
        {
            return false;
        }
    }
    
    // All required tags were found in BoolStorage. We are done.
    return true;
}
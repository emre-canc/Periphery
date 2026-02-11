// Periphery -- EvEGames
#include "Missions/Objectives/Objective_Checklist.h"
#include "Subsystems/MissionSubsystem.h"

void UObjective_Checklist::InitializeRuntime(FObjectiveRuntimeState& RuntimeState) const
{
    Super::InitializeRuntime(RuntimeState);
    RuntimeState.ObjectiveState = EProgressState::InProgress;

    if (bCountPastEvents)
    {
        // 1. Get the Subsystem
        // (You'll need to reach out to GameInstance here, or pass it in)
        UMissionSubsystem* MissionSys = GetWorld()->GetGameInstance()->GetSubsystem<UMissionSubsystem>();
        if (MissionSys)
        {
            for (const FGameplayTag& Tag : RequiredTags)
            {
                int32 EventCount = MissionSys->GetEventCount(Tag);
                
                // 4. Instant Completion Check
                if (EventCount >= 1)
                {
                    // 2. Mark this specific tag as done
                    // Key: "TagName" -> true
                    FName Key = Tag.GetTagName();
                    
                    if (!RuntimeState.BoolStorage.FindRef(Key))
                    {
                        RuntimeState.BoolStorage.Add(Key, true);
                    }
                }
            }
        }
    }
}

bool UObjective_Checklist::OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, 
        AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const
{
    // 1. Is this event in our required list?
    if (RequiredTags.Contains(EventTag))
    {
        // 2. Mark this specific tag as done
        // Key: "TagName" -> true
        FName Key = EventTag.GetTagName();
        
        if (!RuntimeState.BoolStorage.FindRef(Key))
        {
            RuntimeState.BoolStorage.Add(Key, true);
            return true; // State changed
        }
    }
    return false;
}

bool UObjective_Checklist::IsComplete(const FObjectiveRuntimeState& RuntimeState) const
{
    // Check if every required tag is marked true in storage
    for (const FGameplayTag& Tag : RequiredTags)
    {
        if (!RuntimeState.BoolStorage.FindRef(Tag.GetTagName()))
        {
            return false; // Found one missing
        }
    }
    return true;
}
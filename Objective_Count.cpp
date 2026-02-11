
#include "Missions/Objectives/Objective_Count.h"
#include "Subsystems/MissionSubsystem.h"


void UObjective_Count::InitializeRuntime(FObjectiveRuntimeState& RuntimeState) const
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
            int32 EventCount = MissionSys->GetEventCount(TargetEvent);

            RuntimeState.IntStorage.Add("Count", EventCount);
            
            // 4. Instant Completion Check
            if (EventCount >= TargetCount)
            {
                // Logic to mark complete immediately will happen on next tick 
                // or you can force a check here if your architecture allows.
            }
        }
    }
}

bool UObjective_Count::OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, 
    AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const 
{
    if (!EventTag.MatchesTag(TargetEvent)) return false;

    // Unique Check
    if (bRequireUniqueSources && IsValid(SourceActor))
    {
        FString ActorKey = SourceActor->GetName();
        if (RuntimeState.StringStorage.Contains(ActorKey)) return false;
        
        RuntimeState.StringStorage.Add(ActorKey);
    }

    // Increment
    FName Key("Count");
    int32 Val = RuntimeState.IntStorage.FindRef(Key);
    RuntimeState.IntStorage.Add(Key, ++Val);

    return true;
}

bool UObjective_Count::IsComplete(const FObjectiveRuntimeState& RuntimeState) const 
{
    return RuntimeState.IntStorage.FindRef("Count") >= TargetCount;
}
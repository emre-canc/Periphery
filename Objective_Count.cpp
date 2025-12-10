
#include "Missions/Objectives/Objective_Count.h"

bool UObjective_Count::OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, 
    AActor* SourceActor, FObjectiveRuntimeState& ObjectiveRuntime) const 
{
    if (!EventTag.MatchesTag(TargetEvent)) return false;

    // Unique Check
    if (bRequireUniqueSources && IsValid(SourceActor))
    {
        FString ActorKey = SourceActor->GetName();
        if (ObjectiveRuntime.StringStorage.Contains(ActorKey)) return false;
        
        ObjectiveRuntime.StringStorage.Add(ActorKey);
    }

    // Increment
    FName Key("Count");
    int32 Val = ObjectiveRuntime.IntStorage.FindRef(Key);
    ObjectiveRuntime.IntStorage.Add(Key, ++Val);

    return true;
}

bool UObjective_Count::IsComplete(const FObjectiveRuntimeState& ObjectiveRuntime) const 
{
    return ObjectiveRuntime.IntStorage.FindRef("Count") >= TargetCount;
}
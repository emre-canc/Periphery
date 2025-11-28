
#include "Objective_Count.h"


virtual bool OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, 
    AActor* SourceActor, FObjectiveRuntimeState& ObjectiveRuntime) const override
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

virtual bool IsComplete(const FObjectiveRuntimeState& ObjectiveRuntime) const override
{
    return ObjectiveRuntime.IntStorage.FindRef("Count") >= TargetCount;
}
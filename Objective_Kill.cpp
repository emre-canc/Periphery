
#include "Missions/Objectives/Objective_Kill.h"


bool UObjective_Kill::OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, 
    AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const
{
    // 1. Tag Check
    if (!EventTag.MatchesTag(EnemyDeathTag)) return false;

    // 2. Source Check (The reason this class exists)
    if (bRequirePlayerSource)
    {
        // If the killer (SourceActor) isn't the player, ignore it.
        // (Assuming your PlayerController or Pawn has this interface or tag)
        if (!IsValid(SourceActor) || !SourceActor->ActorHasTag("Player")) 
        {
            return false; 
        }
    }

    // 3. Increment (Same generic storage as Count)
    FName Key("KillCount");
    int32 CurrentKills = RuntimeState.IntStorage.FindRef(Key);
    
    RuntimeState.IntStorage.Add(Key, ++CurrentKills);
    
    return true;
}

bool UObjective_Kill::IsComplete(const FObjectiveRuntimeState& RuntimeState) const 
{
    return RuntimeState.IntStorage.FindRef("KillCount") >= RequiredKills;
}
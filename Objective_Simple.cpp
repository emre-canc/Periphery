// Fill out your copyright notice in the Description page of Project Settings.


#include "Missions/Objectives/Objective_Simple.h"

bool UObjective_Simple::OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const
{
    if (EventTag.MatchesTag(TargetEvent))
    {
        RuntimeState.BoolStorage.Add("IsDone", true);
        return true;
    }
    return false;
}

bool UObjective_Simple::IsComplete(const FObjectiveRuntimeState& RuntimeState) const
{
    return RuntimeState.BoolStorage.FindRef("IsDone");
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "Objective_Simple.h"

virtual bool OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const override
{
    if (EventTag.MatchesTag(TargetEvent))
    {
        RuntimeState.BoolStorage.Add("IsDone", true);
        return true;
    }
    return false;
}

virtual bool IsComplete(const FObjectiveRuntimeState& RuntimeState) const override
{
    return RuntimeState.BoolStorage.FindRef("IsDone");
}
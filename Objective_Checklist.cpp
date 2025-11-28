// Fill out your copyright notice in the Description page of Project Settings.


#include "Objective_Checklist.h"

virtual bool OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, 
        AActor* SourceActor, FObjectiveRuntimeState& ObjectiveRuntime) const override
{
    // 1. Is this event in our required list?
    if (RequiredTags.Contains(EventTag))
    {
        // 2. Mark this specific tag as done
        // Key: "TagName" -> true
        FName Key = EventTag.GetTagName();
        
        if (!ObjectiveRuntime.BoolStorage.FindRef(Key))
        {
            ObjectiveRuntime.BoolStorage.Add(Key, true);
            return true; // State changed
        }
    }
    return false;
}

virtual bool IsComplete(const FObjectiveRuntimeState& ObjectiveRuntime) const override
{
    // Check if every required tag is marked true in storage
    for (const FGameplayTag& Tag : RequiredTags)
    {
        if (!ObjectiveRuntime.BoolStorage.FindRef(Tag.GetTagName()))
        {
            return false; // Found one missing
        }
    }
    return true;
}
// Fill out your copyright notice in the Description page of Project Settings.

#include "Missions/Objectives/Objective_Sequence.h"


bool UObjective_Sequence::OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, 
    AActor* SourceActor, FObjectiveRuntimeState& ObjectiveRuntime) const 
{
    // 1. Get Current Step Index (Default 0)
    FName IndexKey("CurrentStepIndex");
    int32 Index = ObjectiveRuntime.IntStorage.FindRef(IndexKey);

    if (!Steps.IsValidIndex(Index)) return false;

    const FObjectiveStepDefinition& CurrentStep = Steps[Index];
    bool bStepProgressed = false;

    // 2. Check Requirements for the CURRENT step only
    for (const FStepRequirement& Req : CurrentStep.RequiredEventsToCompleteStep)
    {
        if (EventTag.MatchesTagExact(Req.RequiredEventTag))
        {
            // -- Unique Source Logic --
            bool bUniqueSource = Req.RequireUniqueSources && IsValid(SourceActor);
            if (bUniqueSource)
            {
                // Key: "StepID_Event_ActorName"
                FString UniqueKey = FString::Printf(TEXT("%s_%s_%s"), 
                    *CurrentStep.StepID.ToString(), 
                    *Req.RequiredEventTag.ToString(), 
                    *SourceActor->GetName());

                if (ObjectiveRuntime.StringStorage.Contains(UniqueKey)) continue; // Already counted
                
                ObjectiveRuntime.StringStorage.Add(UniqueKey);
            }

            // -- Increment Counter --
            // Key: "StepID_EventTag_Count"
            FString CountKeyStr = FString::Printf(TEXT("%s_%s_Count"), 
                *CurrentStep.StepID.ToString(), 
                *Req.RequiredEventTag.ToString());
            FName CountKey(*CountKeyStr);

            int32 CurrentCount = ObjectiveRuntime.IntStorage.FindRef(CountKey);
            ObjectiveRuntime.IntStorage.Add(CountKey, ++CurrentCount);
            bStepProgressed = true;
        }
    }

    if (!bStepProgressed) return false;

    // 3. Check if Step is Complete
    bool bStepComplete = true;
    for (const FStepRequirement& Req : CurrentStep.RequiredEventsToCompleteStep)
    {
        FString CountKeyStr = FString::Printf(TEXT("%s_%s_Count"), 
            *CurrentStep.StepID.ToString(), 
            *Req.RequiredEventTag.ToString());
        
        if (ObjectiveRuntime.IntStorage.FindRef(*CountKeyStr) < Req.NumberOfTimesEventMustOccur)
        {
            bStepComplete = false;
            break;
        }
    }

    if (bStepComplete)
    {
        // 1. Run Complete Actions for the OLD step
        RunStepActions(CurrentStep.StepCompleteActions, SourceActor);

        // 2. Advance Index
        int32 NextIndex = Index + 1;
        ObjectiveRuntime.IntStorage.Add(IndexKey, NextIndex);

        // 3. Run Start Actions for the NEW step (if it exists)
        if (Steps.IsValidIndex(NextIndex))
        {
            RunStepActions(Steps[NextIndex].StepStartActions, SourceActor);
        }

        return true; 
    }
    return false;
}


bool UObjective_Sequence::IsComplete(const FObjectiveRuntimeState& ObjectiveRuntime) const 
{
    // Done if Index has moved past the last step
    int32 Index = ObjectiveRuntime.IntStorage.FindRef("CurrentStepIndex");
    return Index >= Steps.Num();
}


void UObjective_Sequence::RunStepActions(const TArray<TObjectPtr<UMissionAction>>& Actions, AActor* Context) const
{
    for (const UMissionAction* Action : Actions)
    {
        if (Action) Action->ExecuteAction(Context);
    }
}


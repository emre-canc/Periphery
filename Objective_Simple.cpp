// EveGames - Periphery
#include "Missions/Objectives/Objective_Simple.h"
#include "Subsystems/MissionSubsystem.h"

void UObjective_Simple::InitializeRuntime(FObjectiveRuntimeState& RuntimeState) const
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
            
            // 4. Instant Completion Check
            if (EventCount >= 1)
            {
                // Logic to mark complete immediately will happen on next tick 
                // or you can force a check here if your architecture allows.
            }
        }
    }
}

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
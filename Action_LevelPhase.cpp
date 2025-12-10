

#include "Missions/Actions/Action_LevelPhase.h"

void UAction_LevelPhase::ExecuteAction(AActor* ContextActor) const
{
    if (!ContextActor) return;

    // Get the World from the Actor
    UWorld* World = ContextActor->GetWorld();
    if (!World) return;

    // Access the Subsystem from the WORLD

}
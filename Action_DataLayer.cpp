#include "Missions/Actions/Action_DataLayer.h"
#include "Engine/World.h"

void UAction_DataLayer::ExecuteAction(AActor* ContextActor) const
{
    if (!ContextActor) return;

    UWorld* World = ContextActor->GetWorld();
    if (!World) return;

    // Access the Subsystem from the WORLD (because Data Layers are a World concept)
    if (auto* LevelState = World->GetSubsystem<ULevelStateSubsystem>())
    {
        LevelState->SetDataLayerState(LayerName, TargetState);
    }
}
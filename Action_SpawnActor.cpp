
#include "Missions/Actions/Action_SpawnActor.h"
#include "Subsystems/ActorRegistrySubsystem.h"

void UAction_SpawnActor::ExecuteAction(AActor* ContextActor) const
{
    if (!ContextActor || !ActorClass)
    {
        return;
    }

    UWorld* World = ContextActor->GetWorld();
    if (!World)
    {
        return;
    }

    FTransform FinalTransform = SpawnTransform;

    // 1. Resolve Location Strategy
    // If we provided a Tag, we want to spawn relative to that actor (Anchor).
    if (SpawnAtActorTag.IsValid())
    {
        if (auto* Registry = ContextActor->GetGameInstance()->GetSubsystem<UActorRegistrySubsystem>())
        {
            if (AActor* Anchor = Registry->FindActor(SpawnAtActorTag))
            {
                // Compose: Apply our SpawnTransform as an offset to the Anchor's transform.
                // Result = Offset * AnchorWorldTransform
                FinalTransform = SpawnTransform * Anchor->GetActorTransform();
            }
        }
    }

    // 2. Spawn
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = CollisionMethod;
    // We don't set Owner/Instigator here, but you could if needed.

    World->SpawnActor<AActor>(ActorClass, FinalTransform, SpawnParams);
}
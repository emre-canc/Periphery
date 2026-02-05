
#pragma once

#include "CoreMinimal.h"
#include "Missions/Actions/MissionAction.h"
#include "Action_SpawnActor.generated.h"

/**
 * Spawns an actor at a specific transform, or relative to another Tagged Actor.
 */
UCLASS(DisplayName = "Spawn Actor")
class INSIDETFV03_API UAction_SpawnActor : public UMissionAction
{
    GENERATED_BODY()

public:

    // The class to spawn (Enemy, Pickup, etc)
    UPROPERTY(EditAnywhere, Category = "Config")
    TSubclassOf<AActor> ActorClass;

    // Optional: If set, we find this actor (via Registry) and spawn at its location.
    // Useful for "Location.SpawnPoint.Kitchen"
    UPROPERTY(EditAnywhere, Category = "Config")
    FGameplayTag SpawnAtActorTag;

    // The transform.
    // If SpawnAtActorTag is valid, this is applied as a Local Offset.
    // If SpawnAtActorTag is empty, this is the absolute World Transform.
    UPROPERTY(EditAnywhere, Category = "Config")
    FTransform SpawnTransform;

    UPROPERTY(EditAnywhere, Category = "Config")
    ESpawnActorCollisionHandlingMethod CollisionMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    virtual void ExecuteAction(AActor* ContextActor) const override;
};
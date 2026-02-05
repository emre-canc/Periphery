#include "Missions/Actions/Action_SendData.h"
#include "Subsystems/ActorRegistrySubsystem.h"
#include "Interfaces/ConfigurableInterface.h"

void UAction_SendData::ExecuteAction(AActor* ContextActor) const
{
    if (!ContextActor)
    {
        return;
    }


    UActorRegistrySubsystem* Registry = ContextActor->GetGameInstance()->GetSubsystem<UActorRegistrySubsystem>();
    if (!Registry)
    {
        return;
    }

    // Find the target(s). Use GetActors (Plural) in case we want to update all screens at once.
    // If you only expect one, this loop just runs once.
    TArray<AActor*> Targets = Registry->GetActors(ActorTag);

    for (AActor* Target : Targets)
    {
        if (IsValid(Target) && Target->Implements<UConfigurableInterface>())
        {

            IConfigurableInterface::Execute_ReceiveConfiguration(Target, Payload, ContextTag);
        }
    }
}
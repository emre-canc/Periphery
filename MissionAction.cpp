#include "Missions/Actions/MissionAction.h"


void UAction_SendCommand::ExecuteAction(AActor* ContextActor) const
{
    if (!ContextActor || !TargetActorTag.IsValid()) return;

    // We still use the Subsystem's helper because it owns the Actor Registry (TagToActors)
    if (auto* RegisterySys = ContextActor->GetGameInstance()->GetSubsystem<UActorRegistrySubsystem>())
    {
        RegisterySys->SendCommandToActor(TargetActorTag, CommandName);
    }
}

void UAction_Sequence::ExecuteAction(AActor* ContextActor) const
{
    if (!ContextActor || !SequenceTag.IsValid()) return;

    if (auto* RegistrySys = ContextActor->GetGameInstance()->GetSubsystem<UActorRegistrySubsystem>())
    {
        RegistrySys->SendCommandToActor(SequenceTag, "PlayOrStopSequence");
    }
}
#include "Missions/Actions/Action_SendCommand.h"
#include "Subsystems/ActorRegistrySubsystem.h"
#include "Interfaces/CommandInterface.h"



void UAction_SendCommand::ExecuteAction(AActor* ContextActor) const
{
    if (!ActorTag.IsValid() || !CommandTag.IsValid()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("SendCommand ERROR - Invalid Tag or Command"));
        return;
    }

    else if (auto* Registry = ContextActor->GetGameInstance()->GetSubsystem<UActorRegistrySubsystem>())
    {
        TArray<AActor*> Targets = Registry->GetActors(ActorTag);

        for (AActor* Target : Targets)
        {
            // 1. Check for Interface Implementation
            if (Target && Target->Implements<UCommandInterface>())
            {
                // 2. Execute the Interface call safely
                ICommandInterface::Execute_ReceiveCommand(Target, CommandTag);
            }
            else if (Target)
            {
                UE_LOG(LogTemp, Verbose, TEXT("SendCommand ERROR - Actor %s found for tag %s but does not implement CommandInterface."), *Target->GetName(), *ActorTag.ToString());
            }
        }
    }
}
    

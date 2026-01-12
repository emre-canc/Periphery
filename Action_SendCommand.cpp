#include "Missions/Actions/Action_SendCommand.h"
#include "Interfaces/CommandInterface.h"



void UAction_SendCommand::ExecuteAction(AActor* ContextActor) const
{
    if (!ActorTag.IsValid() || !CommandTag.IsValid()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("SendCommand ERROR - Invalid Tag or Command"));
        return;
    }

    else if (auto* Reg = ContextActor->GetGameInstance()->GetSubsystem<UActorRegistrySubsystem>())
    {
        AActor* A = Reg->FindActor(ActorTag);

        // 1. Check for Interface Implementation
        if (A && A->Implements<UCommandInterface>())
        {
            // 2. Execute the Interface call safely
            ICommandInterface::Execute_HandleCommand(A, CommandTag);
        }
        else if (A)
        {
            UE_LOG(LogTemp, Verbose, TEXT("SendCommand ERROR - Actor %s found for tag %s but does not implement CommandInterface."), *A->GetName(), *ActorTag.ToString());
        }
    }
}
    

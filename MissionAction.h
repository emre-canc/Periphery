#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
// #include "Subsystems/ActorRegistrySubsystem.h" //maybe not needed
#include "MissionAction.generated.h"


// Abstract base for all actions (Start, Complete, Step Actions)
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class INSIDETFV03_API UMissionAction : public UObject
{
    GENERATED_BODY()

public:
    // The main entry point. 
    // We pass ContextActor (Player Character) so the Action can find the World/GameInstance.
    virtual void ExecuteAction(AActor* ContextActor) const {};
};





#pragma once

#include "CoreMinimal.h"
#include "Subsystems/ActorRegistrySubsystem.h"
#include "Missions/Actions/MissionAction.h"
#include "Action_SendCommand.generated.h"


UCLASS(DisplayName = "Send Command To Actor")
class INSIDETFV03_API UAction_SendCommand : public UMissionAction
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Config")
    FGameplayTag ActorTag;

    UPROPERTY(EditAnywhere, Category = "Config")
    FGameplayTag CommandTag;

    virtual void ExecuteAction(AActor* ContextActor) const override;
	
};




#pragma once

#include "CoreMinimal.h"
#include "Missions/Actions/MissionAction.h"
#include "Subsystems/LevelStateSubsystem.h"
#include "Action_DataLayer.generated.h"



UCLASS(DisplayName = "Set Data Layer - WIP")
class INSIDETFV03_API UAction_DataLayer : public UMissionAction
{
    GENERATED_BODY()

public:


    virtual void ExecuteAction(AActor* ContextActor) const override;

};

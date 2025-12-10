
#pragma once

#include "CoreMinimal.h"
#include "Missions/Actions/MissionAction.h"
#include "Subsystems/LevelStateSubsystem.h"
#include "Action_LevelPhase.generated.h"



UCLASS(DisplayName = "Set Level Phase - WIP")
class INSIDETFV03_API UAction_LevelPhase : public UMissionAction
{
    GENERATED_BODY()

public:


    virtual void ExecuteAction(AActor* ContextActor) const override;

};

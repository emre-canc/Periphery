#pragma once

#include "CoreMinimal.h"
#include "Missions/Actions/MissionAction.h"
#include "Subsystems/LevelStateSubsystem.h"
#include "Action_DataLayer.generated.h"

/**
 * Mission Action to Toggle World Partition Data Layers.
 * Uses ULevelStateSubsystem to handle the logic.
 */
UCLASS(DisplayName = "Set Data Layer State")
class INSIDETFV03_API UAction_DataLayer : public UMissionAction
{
    GENERATED_BODY()

public:

    // The Short Name of the Data Layer (e.g. "DL_Hell_Staircase")
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    FName LayerName;

    // True = Load/Activate. False = Unload.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    EDataLayerRuntimeState TargetState;

    virtual void ExecuteAction(AActor* ContextActor) const override;
};
#pragma once

#include "CoreMinimal.h"
#include "Missions/Actions/MissionAction.h"
#include "Action_SaveGame.generated.h"

/**
 * Triggers a full game save via the Game Instance.
 */
UCLASS(DisplayName = "Save Game")
class INSIDETFV03_API UAction_SaveGame : public UMissionAction
{
    GENERATED_BODY()

public:
    // Default to "AutoSave" so you don't have to type it every time
    UPROPERTY(EditAnywhere, Category = "Config")
    FString SlotName = "AutoSave"; 

    virtual void ExecuteAction(AActor* ContextActor) const override;
};
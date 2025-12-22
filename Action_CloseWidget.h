#pragma once

#include "CoreMinimal.h"
#include "Missions/Actions/MissionAction.h"
#include "Action_CloseWidget.generated.h"

UCLASS(DisplayName = "Close Widget")
class INSIDETFV03_API UAction_CloseWidget : public UMissionAction
{
    GENERATED_BODY()

public:
    // The Tag assigned to the widget when it was created
    UPROPERTY(EditAnywhere, Category = "Config")
    FName TargetWidgetTag;

    virtual void ExecuteAction(AActor* ContextActor) const override;
};

#pragma once
#include "CoreMinimal.h"
#include "Missions/Actions/MissionAction.h"
#include "Widget/WidgetConfig.h"
#include "Blueprint/UserWidget.h"
#include "Action_CreateWidget.generated.h"

UCLASS(DisplayName = "Create Widget")
class INSIDETFV03_API UAction_CreateWidget : public UMissionAction
{
    GENERATED_BODY()

public:
    // This is the magic dropdown!
    // Selecting a different Payload class here changes the variables below it.
    UPROPERTY(EditAnywhere, Instanced, Category = "Config")
    TObjectPtr<UWidgetConfig> WidgetData;

    UPROPERTY(EditAnywhere, Category = "Config")
    bool bAddToViewport = true;

    // Declaration
    virtual void ExecuteAction(AActor* ContextActor) const override;
};
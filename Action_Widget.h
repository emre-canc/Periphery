
#pragma once

#include "CoreMinimal.h"
#include "Missions/Actions/MissionAction.h"
#include "Widget/WidgetSubsystem.h" 
#include "Action_Widget.generated.h"



UCLASS(DisplayName = "Toggle Widget - WIP")
class INSIDETFV03_API UAction_Widget : public UMissionAction
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Config")
    FGameplayTag WidgetTag;

    virtual void ExecuteAction(AActor* ContextActor) const override;

};

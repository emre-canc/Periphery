#pragma once
#include "CoreMinimal.h"
#include "Missions/Actions/MissionAction.h"
#include "Widget/WidgetConfig.h"
#include "Widget/WidgetStructs.h"
#include "Action_CreateWidget.generated.h"

UCLASS(DisplayName = "Create Widget")
class INSIDETFV03_API UAction_CreateWidget : public UMissionAction
{
    GENERATED_BODY()

public:
    // The Data Asset (Content: Text, Images, etc.)
    UPROPERTY(EditAnywhere, Instanced, Category = "Config")
    TObjectPtr<UWidgetConfig> WidgetConfig;

    // --- WINDOW RULES (Context: How does it behave?) ---

    UPROPERTY(EditAnywhere, Category = "Data")
    EWidgetLayer Layer = EWidgetLayer::Menu;

    UPROPERTY(EditAnywhere, Category = "Data")
    EWidgetInputMode InputMode = EWidgetInputMode::UIOnly;

    UPROPERTY(EditAnywhere, Category = "Data")
    bool bShowMouseCursor = true;

    UPROPERTY(EditAnywhere, Category = "Data")
    bool bPauseGame = false;
    
    // Useful for finding/closing this specific widget later
    UPROPERTY(EditAnywhere, Category = "Data")
    FName ContextTag = NAME_None;

    virtual void ExecuteAction(AActor* ContextActor) const override;
};
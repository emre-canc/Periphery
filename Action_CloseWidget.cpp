#include "Missions/Actions/Action_CloseWidget.h"
#include "Subsystems/WidgetSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UAction_CloseWidget::ExecuteAction(AActor* ContextActor) const
{
    if (TargetWidgetTag.IsNone() || !ContextActor) return;

    UWorld* World = ContextActor->GetWorld();
    if (!World) return;

    UGameInstance* GI = World->GetGameInstance();
    UWidgetSubsystem* WidgetSubsystem = GI ? GI->GetSubsystem<UWidgetSubsystem>() : nullptr;

    if (WidgetSubsystem)
    {
        // 1. Find the specific widget instance by tag
        UUserWidget* WidgetToRemove = WidgetSubsystem->FindWidgetByTag(TargetWidgetTag);

        // 2. Unregister it properly
        // This automatically handles Unpausing, removing from Stack, and Z-Order cleanup
        if (WidgetToRemove)
        {
            WidgetSubsystem->UnregisterWidget(WidgetToRemove);
        }
    }
}
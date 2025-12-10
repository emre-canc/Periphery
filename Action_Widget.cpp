
#include "Missions/Actions/Action_Widget.h"

void UAction_Widget::ExecuteAction(AActor* ContextActor) const 
{
    if (!ContextActor) return;

    if (UWidgetSubsystem* WidgetSys = ContextActor->GetGameInstance()->GetSubsystem<UWidgetSubsystem>())
    {
    
    }
}
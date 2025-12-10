#include "Missions/Actions/Action_CreateWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"

void UAction_CreateWidget::ExecuteAction(AActor* ContextActor) const
{
    if (!ContextActor || !WidgetData) return;

    // 1. Get Player Controller (Required to own a widget)
    APlayerController* PC = nullptr;
    if (APawn* Pawn = Cast<APawn>(ContextActor))
    {
        PC = Cast<APlayerController>(Pawn->GetController());
    }
    
    // Fallback: If Context is not a pawn, try getting first local player
    if (!PC && ContextActor->GetWorld())
    {
        PC = ContextActor->GetWorld()->GetFirstPlayerController();
    }

    if (!PC) return;

    // 2. Get Class from Payload
    TSubclassOf<UUserWidget> ClassToSpawn = WidgetData->GetWidgetClass();
    if (!ClassToSpawn) return;

    // 3. Create Widget
    UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, ClassToSpawn);
    if (NewWidget)
    {
        // 4. THE MAGIC: Let the Payload apply its specific data
        WidgetData->InitializeWidget(NewWidget);

        if (bAddToViewport)
        {
            NewWidget->AddToViewport();
        }
    }
}
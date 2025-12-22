#include "Missions/Actions/Action_CreateWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/WidgetSubsystem.h" 

void UAction_CreateWidget::ExecuteAction(AActor* ContextActor) const
{
    // 1. Validation
    if (!WidgetConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("Action_CreateWidget: WidgetConfig Payload is null."));
        return;
    }

    // 2. Get Player Controller
    APlayerController* PC = nullptr;
    if (APawn* Pawn = Cast<APawn>(ContextActor))
    {
        PC = Cast<APlayerController>(Pawn->GetController());
    }
    // Fallback logic
    if (!PC && ContextActor)
    {
        if (UWorld* World = ContextActor->GetWorld())
        {
            PC = World->GetFirstPlayerController();
        }
    }

    if (!PC) return;

    // 3. Get Class from Payload (WidgetConfig)
    TSubclassOf<UUserWidget> ClassToSpawn = WidgetConfig->GetWidgetClass();
    if (!ClassToSpawn) 
    {
        UE_LOG(LogTemp, Error, TEXT("Action_CreateWidget: WidgetConfig has no Widget Class selected!"));
        return;
    }

    // 4. Get Subsystem
    UGameInstance* GI = PC->GetGameInstance();
    UWidgetSubsystem* WidgetSubsystem = GI ? GI->GetSubsystem<UWidgetSubsystem>() : nullptr;

    if (!WidgetSubsystem) return;

    // 5. Create Widget
    UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, ClassToSpawn);

    if (NewWidget)
    {
        // 6. Apply Payload Content (The Text/Images from your BP Struct)
        WidgetConfig->InitializeWidget(NewWidget);

        // 7. Register with Subsystem (Using the Rules from this Action)
        WidgetSubsystem->RegisterWidget(
            NewWidget, 
            Layer,              
            InputMode,          
            bShowMouseCursor,   
            bPauseGame,         
            ContextTag          
        );
    }
}
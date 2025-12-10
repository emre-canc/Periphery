#include "Widget/WidgetSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

// Hard-coded Z-Orders to prevent "Diversion Squeezing" (Overlaps)
namespace WidgetZOrder
{
    const int32 Background = -10;
    const int32 Overlay    = 10;  // HUD
    const int32 Menu       = 50;  // Inventory, Pause
    const int32 Modal      = 100; // Popups
    const int32 Critical   = 200; // Loading Screens
}

void UWidgetSubsystem::RegisterWidget(UUserWidget* Widget, EWidgetType Type, EWidgetMode Mode, 
    EWidgetProgression Progression, EWidgetPriority Priority, FName ContextTag)
{
    if (!Widget) return;

    // 1. Prevent duplicates
    if (IsWidgetRegistered(Widget))
    {
        UE_LOG(LogTemp, Warning, TEXT("WidgetSubsystem: Widget [%s] already registered."), *Widget->GetName());
        return;
    }

    // 2. Determine Z-Order based on Priority
    int32 ZOrder = WidgetZOrder::Overlay;
    switch (Priority)
    {
        case EWidgetPriority::Low:     ZOrder = WidgetZOrder::Overlay; break;
        case EWidgetPriority::Medium:  ZOrder = WidgetZOrder::Menu; break;
        case EWidgetPriority::High:    ZOrder = WidgetZOrder::Modal; break;
        case EWidgetPriority::Urgent:  ZOrder = WidgetZOrder::Critical; break;
    }

    // 3. Add to Viewport (Enforcing Layering)
    if (!Widget->IsInViewport())
    {
        Widget->AddToViewport(ZOrder);
    }

    // 4. Store Data
    FWidgetData NewData;
    NewData.Widget = Widget;
    NewData.Type = Type;
    NewData.Mode = Mode;
    NewData.Progression = Progression;
    NewData.Priority = Priority;
    NewData.ContextTag = ContextTag;

    ActiveWidgets.Add(Widget, NewData);

    // 5. Special Handling
    
    // If this is an Overlay/HUD, store it reference so we can hide it later
    if (Mode == EWidgetMode::Overlay)
    {
        HUDWidget = Widget;
    }
    // If this is a Menu or Modal, add to the LIFO Stack
    else if (Mode == EWidgetMode::Menu || Mode == EWidgetMode::Modal)
    {
        MenuStack.Add(Widget);
    }

    // 6. Recalculate Input/Pause/Visibility
    RefreshState();

    // Notify listeners
    if (OnWidgetRegistered.IsBound()) OnWidgetRegistered.Broadcast(NewData);
}

void UWidgetSubsystem::UnregisterWidget(UUserWidget* Widget)
{
    if (!Widget || !ActiveWidgets.Contains(Widget)) return;

    FWidgetData OldData = ActiveWidgets[Widget];

    // 1. Remove from Map and Viewport
    ActiveWidgets.Remove(Widget);
    Widget->RemoveFromParent();

    // 2. Remove from Stack (if it was in there)
    // We use RemoveSingle to just take out this specific instance
    // Note: We cast to TWeakObjectPtr to match the array type
    MenuStack.RemoveSingle(Widget);

    // 3. Recalculate Input/Pause/Visibility
    RefreshState();

    if (OnWidgetUnregistered.IsBound()) OnWidgetUnregistered.Broadcast(OldData);
}

bool UWidgetSubsystem::PopTopWidget()
{
    // If stack is empty, we can't go back
    if (MenuStack.IsEmpty()) return false;

    TWeakObjectPtr<UUserWidget> TopWidget = MenuStack.Last();

    if (TopWidget.IsValid())
    {
        UnregisterWidget(TopWidget.Get());
        return true;
    }
    else
    {
        // Prune invalid widget and try again
        MenuStack.Pop();
        return PopTopWidget();
    }
    

    return false;
}

bool UWidgetSubsystem::GetTopWidget(UUserWidget*& Widget)
{
    if (MenuStack.IsEmpty()) return false;

    UUserWidget* TopWidget = MenuStack.Last().Get();

    if (IsValid(TopWidget))
    {
        Widget = TopWidget;
        return true;
    }
    else
    {
        return false;
    }
}

void UWidgetSubsystem::CloseAllMenus()
{
    // Loop backwards safely to close all stack items
    for (int32 i = MenuStack.Num() - 1; i >= 0; i--)
    {
        if (MenuStack[i].IsValid())
        {
            UnregisterWidget(MenuStack[i].Get());
        }
    }
    
    // Safety clear
    MenuStack.Empty();
    RefreshState();
}

// --- QUERIES ---

bool UWidgetSubsystem::IsWidgetRegistered(UUserWidget* Widget) const
{
    return Widget && ActiveWidgets.Contains(Widget);
}

FWidgetData UWidgetSubsystem::GetWidgetData(UUserWidget* Widget) const
{
    if (IsWidgetRegistered(Widget))
    {
        return ActiveWidgets[Widget];
    }
    return FWidgetData(); // Return empty struct
}

bool UWidgetSubsystem::IsAnyMenuOpen() const
{
    return !MenuStack.IsEmpty();
}


// --- THE BRAIN (Internal State Machine) ---

void UWidgetSubsystem::RefreshState()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // SCENARIO A: STACK IS EMPTY (Just Gameplay)
    if (MenuStack.IsEmpty())
    {
        // 1. Show HUD
        if (HUDWidget) HUDWidget->SetVisibility(ESlateVisibility::Visible);

        // 2. Unpause
        UGameplayStatics::SetGamePaused(GetWorld(), false);

        // 3. Restore Input
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(false);
    }
    // SCENARIO B: WE HAVE MENUS OPEN
    else
    {
        // 1. Hide HUD (to prevent clutter/clicks)
        if (HUDWidget) HUDWidget->SetVisibility(ESlateVisibility::Hidden);

        // 2. Get the Top Widget
        UUserWidget* TopWidget = MenuStack.Last().Get();

        // 3. Determine if we Pause (Only "Menu" mode pauses, "Modal" might not)
        if (TopWidget && ActiveWidgets.Contains(TopWidget))
        {
            EWidgetMode CurrentMode = ActiveWidgets[TopWidget].Mode;
            
            // If it's a full Menu, Pause. If it's just a Modal Popup, maybe don't pause?
            // (You can simplify this to always pause if you prefer)
            bool bShouldPause = (CurrentMode == EWidgetMode::Menu);
            UGameplayStatics::SetGamePaused(GetWorld(), bShouldPause);
        }

        // 4. Set Input Focus to the Top Widget
        FInputModeGameAndUI InputMode;
        if (TopWidget)
        {
            InputMode.SetWidgetToFocus(TopWidget->TakeWidget());
        }
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(true);
    }
}
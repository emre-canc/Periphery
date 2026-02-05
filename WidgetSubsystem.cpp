#include "Subsystems/WidgetSubsystem.h"
#include "Widget/PeripheryWidgetSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

// --- REGISTRATION ---
void UWidgetSubsystem::RegisterWidget(UUserWidget* Widget, EWidgetLayer Layer, EWidgetInputMode InputMode,
    bool bShowMouseCursor, bool bPauseGame, FName ContextTag)
{
    if (!Widget) 
    {
        UE_LOG(LogTemp, Error, TEXT("WidgetSubsystem: RegisterWidget failed! Widget is NULL."));
        return;
    }

    // 1. Prevent duplicates
    if (IsWidgetRegistered(Widget))
    {
        UE_LOG(LogTemp, Warning, TEXT("WidgetSubsystem: [%s] already registered."), *Widget->GetName());
        return;
    }

    // 2. Determine Z-Order based on Layer
    int32 ZOrder = 10; 
    switch (Layer)
    {
        case EWidgetLayer::Game:    ZOrder = 10;  break; 
        case EWidgetLayer::Menu:    ZOrder = 50;  break;
        case EWidgetLayer::Modal:   ZOrder = 100; break;
        case EWidgetLayer::System:  ZOrder = 200; break;
    }

    // 3. Add to Viewport (if not already there)
    if (!Widget->IsInViewport())
    {
        Widget->AddToViewport(ZOrder);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WidgetSubsystem: [%s] already in viewport. Skipping Add."), *Widget->GetName());
    }

    // 4. Store Data
    FWidgetData NewData;
    NewData.Widget = Widget;
    NewData.Layer = Layer;                  // Visual Depth
    NewData.InputMode = InputMode;          // Behavior Rule
    NewData.bShowMouseCursor = bShowMouseCursor; 
    NewData.bPauseGame = bPauseGame; 
    NewData.ContextTag = ContextTag;

    ActiveWidgets.Add(Widget, NewData);

    // 5. Special Handling & Stack Management

    // Stack Logic: If it's NOT "GameOnly", it blocks something, so it goes on the stack.
    if (InputMode != EWidgetInputMode::GameOnly)
    {
        MenuStack.Add(Widget);
        UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: Pushed [%s] to Input Stack."), *Widget->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: [%s] is GameOnly . Skipping Stack."), *Widget->GetName());
    }

    // 6. Refresh State
    RefreshState();

    if (OnWidgetRegistered.IsBound()) OnWidgetRegistered.Broadcast(NewData);
}

void UWidgetSubsystem::UnregisterWidget(UUserWidget* Widget)
{
    if (!Widget || !ActiveWidgets.Contains(Widget)) return;

    FWidgetData OldData = ActiveWidgets[Widget];

    // 1. Remove from Map and Viewport
    ActiveWidgets.Remove(Widget);
    Widget->RemoveFromParent();

    // 2. Remove from Stack (safe even if it wasn't there)
    MenuStack.RemoveSingle(Widget);

    // 3. Refresh State
    RefreshState();

    if (OnWidgetUnregistered.IsBound()) OnWidgetUnregistered.Broadcast(OldData);
}


// --- THE BRAIN ---

void UWidgetSubsystem::RefreshState()
{
    // [DEBUG] Entry Log
    UE_LOG(LogTemp, Warning, TEXT("WidgetSubsystem: RefreshState() Called."));

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("WidgetSubsystem: RefreshState FAILED - No PlayerController."));
        return;
    }

    // --- 1. SETUP ENHANCED INPUT ---
    UEnhancedInputLocalPlayerSubsystem* EISubsystem = nullptr;
    if (ULocalPlayer* LP = PC->GetLocalPlayer())
    {
        EISubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    }

    // --- 2. LOAD IMC FROM SETTINGS ---
    static UInputMappingContext* UI_IMC = nullptr;
    
    // Only try to load if we haven't already
    if (!UI_IMC)
    {
        const UPeripheryUISettings* Settings = GetDefault<UPeripheryUISettings>();
        if (Settings && !Settings->DefaultUIInputContext.IsNull())
        {
            UI_IMC = Settings->DefaultUIInputContext.LoadSynchronous();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("WidgetSubsystem: UI_IMC not set in Project Settings -> Periphery UI Settings!"));
        }
    }

    // [DEBUG] Check Stack Size
    int32 CurrentStackSize = MenuStack.Num();
    UE_LOG(LogTemp, Warning, TEXT("WidgetSubsystem: Current MenuStack Size = %d"), CurrentStackSize);

    // --- SCENARIO A: STACK EMPTY (Gameplay) ---
    if (MenuStack.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: Scenario A (Gameplay) - Stack Empty. Restoring Controls."));

        SetHUDVisibility(true);

        // Unpause
        UGameplayStatics::SetGamePaused(GetWorld(), false);
        UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: Game Unpaused."));

        // Restore Input
        PC->ResetIgnoreMoveInput();
        PC->ResetIgnoreLookInput();
        PC->SetShowMouseCursor(false);
        UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: Input Restored (Move: ON, Look: ON, Mouse Cursor: OFF)."));
        

        // Input Mode
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: SetInputMode -> GameOnly."));

        // Remove UI IMC
        if (EISubsystem && UI_IMC)
        {
            EISubsystem->RemoveMappingContext(UI_IMC);
            UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: UI_IMC has been removed!"));
        }
    }

    // --- SCENARIO B: MENUS OPEN ---
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WidgetSubsystem: Scenario B (UI Mode) - Stack Has Widgets."));

        // Add UI IMC
        if (EISubsystem && UI_IMC)
        {
            EISubsystem->AddMappingContext(UI_IMC, 1); 
            UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: UI_IMC has been added!"));
        }

        // Get Top Widget Data
        UUserWidget* TopWidget = MenuStack.Last().Get();
        FWidgetData Data;
        
        if (TopWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("WidgetSubsystem: Top Widget Name: %s"), *TopWidget->GetName());

            if (ActiveWidgets.Contains(TopWidget))
            {
                Data = ActiveWidgets[TopWidget];
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("WidgetSubsystem: CRITICAL - Top Widget '%s' is in Stack but NOT in ActiveWidgets Map! Data will be default."), *TopWidget->GetName());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("WidgetSubsystem: CRITICAL - Top Widget in Stack is NULL (Pending Kill?)."));
        }

        // Apply Pause
        UGameplayStatics::SetGamePaused(GetWorld(), Data.bPauseGame);
        UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: SetGamePaused -> %s"), Data.bPauseGame ? TEXT("TRUE") : TEXT("FALSE"));

        // Apply Input Blocking (Hard Block)
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
        UE_LOG(LogTemp, Warning, TEXT("WidgetSubsystem: Input Blocked (Move: OFF, Look: OFF)."));

        SetHUDVisibility(false); 

        EMouseLockMode LockMode = Data.bShowMouseCursor ? EMouseLockMode::DoNotLock : EMouseLockMode::LockAlways;

        if (Data.InputMode == EWidgetInputMode::UIOnly)
        {
            UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: Setting InputMode -> UIOnly."));
            FInputModeUIOnly InputMode;
            if (TopWidget  && Data.bShowMouseCursor) InputMode.SetWidgetToFocus(TopWidget->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(LockMode);
            PC->SetInputMode(InputMode);
        }
        else // GameAndUI (Fallback / Interactive)
        {
            UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: Setting InputMode -> GameAndUI."));
            FInputModeGameAndUI InputMode;
            if (TopWidget && Data.bShowMouseCursor) InputMode.SetWidgetToFocus(TopWidget->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(LockMode);
            PC->SetInputMode(InputMode);
        }
        // Apply ShowMouseCursor after SetWidgetToFocus
        PC->SetShowMouseCursor(Data.bShowMouseCursor);
        UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: Mouse Cursor -> %s"), Data.bShowMouseCursor ? TEXT("TRUE") : TEXT("FALSE"));
    }
}



// --- STACK CONTROL ---

bool UWidgetSubsystem::PopTopWidget()
{
    if (MenuStack.IsEmpty()) return false;

    // Remove the last widget added to the stack
    TWeakObjectPtr<UUserWidget> TopWidget = MenuStack.Last();

    if (TopWidget.IsValid())
    {
        UnregisterWidget(TopWidget.Get());
        return true;
    }
    else
    {
        // Clean up invalid entry and try again
        MenuStack.Pop();
        return PopTopWidget();
    }
   
}

void UWidgetSubsystem::CloseAllMenus()
{
    // Iterate backwards to close everything in the stack
    for (int32 i = MenuStack.Num() - 1; i >= 0; i--)
    {
        if (MenuStack[i].IsValid())
        {
            UnregisterWidget(MenuStack[i].Get());
        }
    }
    MenuStack.Empty();
    RefreshState();
}

bool UWidgetSubsystem::CloseWidgetByContext(FName ContextTag)
{
    if (ContextTag.IsNone()) return false;

    UUserWidget* Target = FindWidgetByTag(ContextTag);
    if (Target)
    {
        UnregisterWidget(Target);
        return true;
    }
    return false;
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
    return FWidgetData(); 
}

bool UWidgetSubsystem::IsAnyMenuOpen() const
{
    return !MenuStack.IsEmpty();
}

bool UWidgetSubsystem::GetTopWidget(UUserWidget*& Widget)
{
    if (MenuStack.IsEmpty()) return false;
    
    Widget = MenuStack.Last().Get();
    return IsValid(Widget);
}

UUserWidget* UWidgetSubsystem::FindWidgetByTag(FName Tag) const
{
    if (Tag.IsNone()) return nullptr;

    for (const auto& Pair : ActiveWidgets)
    {
        if (Pair.Value.ContextTag == Tag && Pair.Key.IsValid())
        {
            return Pair.Key.Get();
        }
    }
    return nullptr;
}

void UWidgetSubsystem::SetHUDVisibility(bool bVisible)
{

    ESlateVisibility Visibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

    // Iterate over ALL registered widgets
    for (auto& Pair : ActiveWidgets)
    {
        // If this widget is on the GAME layer (HUD) show it
        if (Pair.Value.Layer == EWidgetLayer::Game)
        {
            if (UUserWidget* HUD = Pair.Key.Get())
            {
                if (HUD->Implements<UWidgetInterface>()) IWidgetInterface::Execute_SetWidgetVisibility(HUD, bVisible);
                else HUD->SetVisibility(Visibility);
            }
        }
    }
}


// --- MENUS ---

void UWidgetSubsystem::OpenMenu(TSubclassOf<UUserWidget> WidgetClass)
{
    if (!WidgetClass) return;

    // 1. Clean up existing menu if open
    CloseMenu();

    // 2. Create the new widget
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        CurrentMenuWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
        RegisterWidget(
                CurrentMenuWidget, 
                EWidgetLayer::Menu,      // Layer
                EWidgetInputMode::UIOnly,// Input Mode
                true,                    // Show Mouse
                false,                   // Pause Game (Optional, Main Menu usually doesn't pause time, just input)
                FName("MainMenu")        // Context Tag
            );
    }
}

void UWidgetSubsystem::CloseMenu()
{
    if (CurrentMenuWidget)
    {
        CurrentMenuWidget->RemoveFromParent();
        CurrentMenuWidget = nullptr;
    }
}
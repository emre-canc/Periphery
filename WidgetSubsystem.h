#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Widget/WidgetStructs.h"
#include "Interfaces/WidgetInterface.h"
#include "WidgetSubsystem.generated.h"

UCLASS()
class INSIDETFV03_API UWidgetSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    // --- CORE ---

    // Registers a widget, calculates Z-Order, pushes to Stack, and hides HUD automatically.
    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem")
    void RegisterWidget(UUserWidget* Widget, EWidgetLayer Layer, EWidgetInputMode InputMode,
        bool bShowMouseCursor, bool bPauseGame, FName ContextTag = NAME_None);

    // Removes widget. If it was a Menu, pops from Stack and checks if HUD should reappear.
    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem")
    void UnregisterWidget(UUserWidget* Widget);

    // --- STACK CONTROL ---

    // Simulates "Back/Escape". Closes the top-most Menu/Modal. Returns true if something closed.
    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem|Input")
    bool PopTopWidget();

    // Force closes all non-HUD widgets (Great for "Return to Main Menu").
    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem|Input")
    void CloseAllMenus();

    // --- QUERIES ---

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem")
    bool IsWidgetRegistered(UUserWidget* Widget) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem")
    FWidgetData GetWidgetData(UUserWidget* Widget) const;
    
    // Check if any Menus/Modals are currently pausing the game
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem")
    bool IsAnyMenuOpen() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem")
    bool GetTopWidget(UUserWidget*& Widget);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem")
    bool CloseWidgetByContext(FName ContextTag);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem")
    UUserWidget* FindWidgetByTag(FName Tag) const;

    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem")
    void SetHUDVisibility(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "WidgetSubsystem|Menus")
    void OpenMenu(TSubclassOf<UUserWidget> WidgetClass);

    UFUNCTION(BlueprintCallable, Category = "WidgetSubsystem|Menus")
    void CloseMenu();



    // --- EVENTS ---

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetEvent, FWidgetData, WidgetData);

    UPROPERTY(BlueprintAssignable, Category="WidgetSubsystem")
    FOnWidgetEvent OnWidgetRegistered;

    UPROPERTY(BlueprintAssignable, Category="WidgetSubsystem")
    FOnWidgetEvent OnWidgetUnregistered;

private:
    
    UPROPERTY()
    TMap<TWeakObjectPtr<UUserWidget>, FWidgetData> ActiveWidgets;
    
    // The LIFO Stack. Index 0 = Bottom.
    UPROPERTY()
    TArray<TWeakObjectPtr<UUserWidget>> MenuStack;

    UPROPERTY()
    TObjectPtr<UUserWidget> CurrentMenuWidget;

    // Internal helper to sync Input Mode/HUD visibility with the Stack state
    void RefreshState(); 

    
};
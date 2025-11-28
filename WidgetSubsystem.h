// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "WidgetSubsystem.generated.h"

class UInputMappingContext;

UENUM(BlueprintType)
enum class EWidgetType : uint8
{
    System     UMETA(DisplayName="System"),
    Gameplay   UMETA(DisplayName="Gameplay"),
    Narrative  UMETA(DisplayName="Narrative"),
    Other      UMETA(DisplayName="Other")
};


UENUM(BlueprintType)
enum class EWidgetMode : uint8
{
    Modal     UMETA(DisplayName="Modal"),
    Overlay   UMETA(DisplayName="Overlay"),
    Menu      UMETA(DisplayName="Menu")
};

UENUM(BlueprintType)
enum class EWidgetProgression : uint8
{
    Manual     UMETA(DisplayName="Manual"),
    Automatic  UMETA(DisplayName="Automatic")
};

UENUM(BlueprintType)
enum class EWidgetPriority : uint8
{
    Low     UMETA(DisplayName="Low"),
    Medium  UMETA(DisplayName="Medium"),
    High    UMETA(DisplayName="High"),
    Urgent  UMETA(DisplayName="Urgent")
};

USTRUCT(BlueprintType)
struct FWidgetData
{
    GENERATED_BODY()

    UPROPERTY(Transient, BlueprintReadOnly) 
    TWeakObjectPtr<UUserWidget> Widget;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere) 
    FName WidgetContext = NAME_None;

    UPROPERTY(BlueprintReadWrite, EditAnywhere) 
    EWidgetType Type = EWidgetType::Other;

    UPROPERTY(BlueprintReadWrite, EditAnywhere) 
    EWidgetMode Mode = EWidgetMode::Overlay;

    UPROPERTY(BlueprintReadWrite, EditAnywhere) 
    EWidgetProgression Progression = EWidgetProgression::Automatic;

    UPROPERTY(BlueprintReadWrite, EditAnywhere) 
    EWidgetPriority Priority = EWidgetPriority::Low;


    //Constructers used for C++, commented because it is not needed but good to have.
    // FWidgetData() {} 
    // FWidgetData(UUserWidget* InWidget, EWidgetType InType)
    //     : Widget(InWidget), Type(InType) {}
};

USTRUCT(BlueprintType)
struct FWidgetDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UUserWidget> WidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EWidgetType Type = EWidgetType::Other;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EWidgetMode Mode = EWidgetMode::Overlay;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EWidgetProgression Progression = EWidgetProgression::Automatic;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EWidgetPriority Priority = EWidgetPriority::Low;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetRegistered,   FWidgetData, WidgetData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetUnregistered, FWidgetData, WidgetData);

UCLASS()
class INSIDETFV03_API UWidgetSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem")
    void RegisterWidget(UUserWidget* Widget, EWidgetType Type, EWidgetMode Mode, 
         EWidgetProgression Progression, EWidgetPriority Priority, FName ContextTag = NAME_None);

    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem")  //for some reason still overlap each other, activewidget doesnt correctly track widgets on screen. 
    void UnregisterWidget(UUserWidget* Widget);               //theres something up with register/unregister i need to make log outputs to be able to debug.


        // Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem|Query")
    FWidgetData GetWidgetData(UUserWidget* Widget) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem|Query")
    TArray<FWidgetData> GetAllByType(EWidgetType Type) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem|Query")
    TArray<FWidgetData> GetAllByContext(FName ContextTag) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem|Query")
    bool IsWidgetTypeShowing(EWidgetType Type) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem|Query")
    bool IsWidgetRegistered(UUserWidget* Widget) const;

        // Modal control
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem|Modal")
    FWidgetData GetModalWidget() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem|Modal")
    bool IsModalWidgetActive() const;

    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem|Modal")
    void ReplaceModalWidget(UUserWidget* Widget, EWidgetType Type, EWidgetMode Mode,
         EWidgetProgression Progression, EWidgetPriority Priority, FName ContextTag = NAME_None);

    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem|Modal")
    void DismissModalWidget(bool bRestoreInput = true);

    UFUNCTION(BlueprintCallable) bool ShowByContext(FGameplayTag ContextTag);
    UFUNCTION(BlueprintCallable) bool HideByContext(FGameplayTag ContextTag);
    UFUNCTION(BlueprintCallable) bool ToggleByContext(FGameplayTag ContextTag);

    // Optional convenience overloads for callers that have names:
    UFUNCTION(BlueprintCallable) bool ShowByContextName(FName ContextName);
    UFUNCTION(BlueprintCallable) bool HideByContextName(FName ContextName);
    UFUNCTION(BlueprintCallable) bool ToggleByContextName(FName ContextName);

    UPROPERTY(EditDefaultsOnly, Category="WidgetSubsystem|Definitions")
    TMap<FName, FWidgetDefinition> WidgetDefinitions;


    /**.  Input & pause management
     * ⚠️ Manual override for input locking.
     * Normally you don’t need this — RegisterWidget / UnregisterWidget
     * handle input automatically for Modals and Menus.
     * 
     * Use only for special cases (e.g. forcing UI input outside widget flow).
     */
    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem|InputDebug")
    void LockInput(FName Reason, bool bShowCursor = false);

    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem|InputDebug")
    void UnlockInput(FName Reason);

    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem|InputDebug")
    void SetPausedForMenu(bool bPaused);


        // Debug
    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem|Debug")
    void DumpStateToLog() const;


      //Miscellaneous
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="WidgetSubsystem|Menu")
    bool IsMenuActive() const;

    UFUNCTION(BlueprintCallable, Category="WidgetSubsystem|Utility")
    void DismissAllWidgets(bool bRestoreInput = true);
    

    UPROPERTY(BlueprintAssignable, Category="WidgetSubsystem|Events")
    FOnWidgetRegistered OnWidgetRegistered;

    UPROPERTY(BlueprintAssignable, Category="WidgetSubsystem|Events")
    FOnWidgetUnregistered OnWidgetUnregistered;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="WidgetSubsystem")
    UUserWidget* HUDWidget;


private:
    // Map of Widgets -> WidgetData
    UPROPERTY()
    TMap<TWeakObjectPtr<UUserWidget>, FWidgetData> ActiveWidgets; //A map where the key is a Widget and that widgets data as the values.
    

    FWidgetData ActiveModal;
    FWidgetData ActiveMenu;

	UPROPERTY() UInputMappingContext* UIInputMappingContext = nullptr;


    // maintenance
    void PruneWidgets();
    void EnsureUIIMCLoaded();
};

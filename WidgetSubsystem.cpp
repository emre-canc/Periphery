// WidgetSubsystem.cpp

#include "WidgetSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputSubsystems.h" 
#include "InputMappingContext.h" 
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogWidgetSubsystem, Log, All);

// --------- Helpers
namespace
{
    struct FInputLockToken
    {
        FName Reason;
        bool bShowCursor = false;
    };

    static TArray<FInputLockToken> GInputLockStack;
}

static bool HasLock(FName Reason)
{
    return GInputLockStack.ContainsByPredicate(
        [&](const FInputLockToken& T){ return T.Reason == Reason; });
}



static void ModalGameplayBlock(UWorld* World)
{
    if (!World) return;
    if (APlayerController* Player = UGameplayStatics::GetPlayerController(World, 0))
    {
        // Block camera & movement input reaching the Pawn
        Player->SetIgnoreLookInput(true);
        Player->SetIgnoreMoveInput(true);
        
        if (APawn* Pawn = Player->GetPawn())
        {
            Pawn->DisableInput(Player);       
        }
    }
}



static FString SafeName(const UObject* Obj)
{
    return IsValid(Obj) ? Obj->GetName() : TEXT("None");
}

static FString ToString(EWidgetType T)
{
    if (const UEnum* Enum = StaticEnum<EWidgetType>())
        return Enum->GetNameStringByValue(static_cast<int64>(T));
    return TEXT("EWidgetType(?)");
}

static FString ToString(EWidgetMode M)
{
    if (const UEnum* Enum = StaticEnum<EWidgetMode>())
        return Enum->GetNameStringByValue(static_cast<int64>(M));
    return TEXT("EWidgetMode(?)");
}

static FString ToString(EWidgetProgression P)
{
    if (const UEnum* Enum = StaticEnum<EWidgetProgression>())
        return Enum->GetNameStringByValue(static_cast<int64>(P));
    return TEXT("EWidgetProgression(?)");
}

static FString ToString(EWidgetPriority Pri)
{
    if (const UEnum* Enum = StaticEnum<EWidgetPriority>())
        return Enum->GetNameStringByValue(static_cast<int64>(Pri));
    return TEXT("EWidgetPriority(?)");
}

static const FName NAME_Modal(TEXT("Modal"));
static const FName NAME_Menu(TEXT("Menu"));

// Dump one entry
static void LogEntry(const FWidgetData& D, int32 Index = -1)
{
    const UUserWidget* W = D.Widget.Get();
    if (Index >= 0)
    {
        UE_LOG(LogWidgetSubsystem, Log, TEXT("  [%d] %s | Type=%s Mode=%s Prog=%s Pri=%s Ctx=%s"),
            Index,
            *SafeName(W),
            *ToString(D.Type),
            *ToString(D.Mode),
            *ToString(D.Progression),
            *ToString(D.Priority),
            *D.WidgetContext.ToString());
    }
    else
    {
        UE_LOG(LogWidgetSubsystem, Log, TEXT("  %s | Type=%s Mode=%s Prog=%s Pri=%s Ctx=%s"),
            *SafeName(W),
            *ToString(D.Type),
            *ToString(D.Mode),
            *ToString(D.Progression),
            *ToString(D.Priority),
            *D.WidgetContext.ToString());
    }
}

static UEnhancedInputLocalPlayerSubsystem* GetEI(UObject* WorldContext)
{
    if (!WorldContext) return nullptr;
    if (APlayerController* Player = UGameplayStatics::GetPlayerController(WorldContext, 0))
        if (ULocalPlayer* LP = Player->GetLocalPlayer())
            return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    return nullptr;
}


// --------- Registration

void UWidgetSubsystem::RegisterWidget(UUserWidget* Widget, EWidgetType Type, EWidgetMode Mode, EWidgetProgression Progression, EWidgetPriority Priority, FName ContextTag /*= NAME_None*/)
{
    if (!Widget)
    {
        UE_LOG(LogWidgetSubsystem, Warning, TEXT("RegisterWidget: Widget=nullptr"));
        return;
    }

    PruneWidgets();

    if (ActiveWidgets.Contains(Widget))
    {
        UE_LOG(LogWidgetSubsystem, Verbose, TEXT("RegisterWidget: %s already registered"), *Widget->GetName());
        return;
    }

    FWidgetData Entry;
    Entry.Widget        = Widget;
    Entry.WidgetContext = ContextTag;
    Entry.Type          = Type;
    Entry.Mode          = Mode;
    Entry.Progression   = Progression;
    Entry.Priority      = Priority;
    OnWidgetRegistered.Broadcast(Entry);

    switch (Mode) {
    // Special handling by Mode

    case EWidgetMode::Menu:
    {
        const UUserWidget* Prev = ActiveMenu.Widget.Get();
        ActiveMenu = Entry;

        // Menu owns input & (optionally) pauses game
        LockInput(TEXT("Menu"), /*bShowCursor=*/true);
        SetPausedForMenu(true);

        UE_LOG(LogWidgetSubsystem, Log, TEXT("RegisterWidget(Menu): %s (prev=%s)"),
         *Widget->GetName(), *SafeName(Prev));
         break;
    }
    case EWidgetMode::Modal:
    {
        const UUserWidget* Prev = ActiveModal.Widget.Get();
        ActiveModal = Entry;

        // Modal owns input; no pause by default
        const bool bShowCursor = (Type == EWidgetType::Gameplay);
        LockInput(TEXT("Modal"), bShowCursor);

        UE_LOG(LogWidgetSubsystem, Log, TEXT("RegisterWidget(Modal): %s (prev=%s)"),
            *Widget->GetName(), *SafeName(Prev));
            break;
    }
    case EWidgetMode::Overlay:
    {
        // Overlay: no input/pause changes

        UE_LOG(LogWidgetSubsystem, Log, TEXT("RegisterWidget(Overlay): %s Type=%s Ctx=%s"),
            *Widget->GetName(), *ToString(Type), *ContextTag.ToString());
            break;
    }
    }
    ActiveWidgets.Add(Widget, Entry);
}



void UWidgetSubsystem::UnregisterWidget(UUserWidget* Widget)
{
    if (!Widget)
    {
        UE_LOG(LogWidgetSubsystem, Warning, TEXT("UnregisterWidget: Widget=nullptr"));
        return;
    }

    PruneWidgets();

    FWidgetData* Found = ActiveWidgets.Find(Widget);
    if (!Found)
    {
        UE_LOG(LogWidgetSubsystem, Verbose, TEXT("UnregisterWidget: %s not found in registry"), *Widget->GetName());
        return;
    }

    const EWidgetMode Mode = Found->Mode;
    const FWidgetData Data = *Found; // copy before removal
    OnWidgetUnregistered.Broadcast(Data);

    // If removing the active modal/menu, clear slots and restore states
    if (Mode == EWidgetMode::Menu && ActiveMenu.Widget.Get() == Widget)
    {
        UE_LOG(LogWidgetSubsystem, Log, TEXT("UnregisterWidget(Menu): %s -> clearing active menu, unpausing & unlocking"),
            *Widget->GetName());
        ActiveMenu = FWidgetData{};
        SetPausedForMenu(false);
        UnlockInput(TEXT("Menu"));
    }
    else if (Mode == EWidgetMode::Modal && ActiveModal.Widget.Get() == Widget)
    {
        UE_LOG(LogWidgetSubsystem, Log, TEXT("UnregisterWidget(Modal): %s -> clearing active modal & unlocking"),
            *Widget->GetName());
        ActiveModal = FWidgetData{};
        UnlockInput(TEXT("Modal"));
    }
    else
    {
        // Overlay or a non-active modal/menu entry
        UE_LOG(LogWidgetSubsystem, Log, TEXT("UnregisterWidget: %s removed (Mode=%s)"),
            *Widget->GetName(), *ToString(Mode));
    }

    ActiveWidgets.Remove(Widget);
}

// --------- Queries

FWidgetData UWidgetSubsystem::GetWidgetData(UUserWidget* Widget) const
{
    FWidgetData Out;
    if (!Widget)
        return Out;

    const FWidgetData* Found = ActiveWidgets.Find(Widget);
    if (Found && Found->Widget.IsValid())
    {
        Out = *Found;
    }
    return Out;
}

TArray<FWidgetData> UWidgetSubsystem::GetAllByType(EWidgetType Type) const
{
    TArray<FWidgetData> Result;
    Result.Reserve(ActiveWidgets.Num());

    for (const auto& KV : ActiveWidgets)
    {
        const FWidgetData& D = KV.Value;
        if (D.Widget.IsValid() && D.Type == Type)
        {
            Result.Add(D);
        }
    }
    return Result;
}

TArray<FWidgetData> UWidgetSubsystem::GetAllByContext(FName ContextTag) const
{
    TArray<FWidgetData> Result;
    Result.Reserve(ActiveWidgets.Num());

    for (const auto& KV : ActiveWidgets)
    {
        const FWidgetData& D = KV.Value;
        if (D.Widget.IsValid() && D.WidgetContext == ContextTag)
        {
            Result.Add(D);
        }
    }
    return Result;
}

bool UWidgetSubsystem::IsWidgetTypeShowing(EWidgetType Type) const
{
    for (const auto& KV : ActiveWidgets)
    {
        const FWidgetData& D = KV.Value;
        if (D.Widget.IsValid() && D.Type == Type)
        {
            return true;
        }
    }
    return false;
}

bool UWidgetSubsystem::IsWidgetRegistered(UUserWidget* Widget) const
{
    if (!Widget) return false;

    const FWidgetData* Found = ActiveWidgets.Find(Widget);
    return Found && Found->Widget.IsValid();
}

// --------- Modal control

FWidgetData UWidgetSubsystem::GetModalWidget() const
{
    return ActiveModal;
}

bool UWidgetSubsystem::IsModalWidgetActive() const
{
    return ActiveModal.Widget.IsValid();
}

void UWidgetSubsystem::ReplaceModalWidget(
    UUserWidget* Widget,
    EWidgetType Type,
    EWidgetMode Mode,
    EWidgetProgression Progression,
    EWidgetPriority Priority,
    FName ContextTag /*= NAME_None*/)
{
    if (!Widget)
    {
        UE_LOG(LogWidgetSubsystem, Warning, TEXT("ReplaceModalWidget: Widget=nullptr"));
        return;
    }

    // Policy: replace current modal with the new one (even if new one is Menu we treat it via Register)
    // First, dismiss existing modal (but do not touch menu here)
    if (ActiveModal.Widget.IsValid())
    {
        UUserWidget* Prev = ActiveModal.Widget.Get();
        UE_LOG(LogWidgetSubsystem, Log, TEXT("ReplaceModalWidget: dismissing existing modal %s"), *SafeName(Prev));
        ActiveWidgets.Remove(Prev); // ensure registry consistency
        ActiveModal = FWidgetData{};
        UnlockInput(TEXT("Modal"));
    }

    // Register the new one as usual
    RegisterWidget(Widget, Type, Mode, Progression, Priority, ContextTag);
}

void UWidgetSubsystem::DismissModalWidget(bool bRestoreInput /*= true*/)
{
    if (!ActiveModal.Widget.IsValid())
    {
        UE_LOG(LogWidgetSubsystem, Verbose, TEXT("DismissModalWidget: no active modal"));
        return;
    }

    UUserWidget* W = ActiveModal.Widget.Get();
    UE_LOG(LogWidgetSubsystem, Log, TEXT("DismissModalWidget: %s"), *SafeName(W));

    // Remove from registry if present
    ActiveWidgets.Remove(W);

    ActiveModal = FWidgetData{};

    if (bRestoreInput)
    {
        UnlockInput(TEXT("Modal"));
    }
}

// --------- Menu / Utility

bool UWidgetSubsystem::IsMenuActive() const
{
    return ActiveMenu.Widget.IsValid();
}

void UWidgetSubsystem::DismissAllWidgets(bool bRestoreInput /*= true*/)
{
    UE_LOG(LogWidgetSubsystem, Log, TEXT("DismissAllWidgets: clearing %d entries"), ActiveWidgets.Num());

    // Best-effort: if you also want to remove from viewport, do it here (optional)
    for (auto& KV : ActiveWidgets)
    {
        if (UUserWidget* W = KV.Key.Get())
        {
            W->RemoveFromParent();
        }
    }

    ActiveWidgets.Empty();

    // Clear modal/menu and restore policies
    if (ActiveModal.Widget.IsValid())
    {
        ActiveModal = FWidgetData{};
        if (bRestoreInput)
        {
            UnlockInput(TEXT("Modal"));
        }
    }

    if (ActiveMenu.Widget.IsValid())
    {
        ActiveMenu = FWidgetData{};
        SetPausedForMenu(false);
        if (bRestoreInput)
        {
            UnlockInput(TEXT("Menu"));
        }
    }
}


bool UWidgetSubsystem::ToggleByContext(FGameplayTag ContextTag)
{
    if (!ContextTag.IsValid()) return false;
    const FName ContextName = ContextTag.GetTagName();

    // If any exist, hide them (toggle off)
    TArray<FWidgetData> Found = GetAllByContext(ContextName);
    if (Found.Num() > 0)
    {
        for (FWidgetData& Data : Found)
        {
            if (UUserWidget* W = Data.Widget.Get())
            {
                W->RemoveFromParent();
                UnregisterWidget(W);
            }
        }
        UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: ToggleByContext -> Hid %s"), *ContextTag.ToString());
        return true;
    }

    // Otherwise show (toggle on)
    const FWidgetDefinition* Def = WidgetDefinitions.Find(ContextName);
    if (!Def || !Def->WidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("WidgetSubsystem: ToggleByContext -> No definition for %s"), *ContextTag.ToString());
        return false;
    }

    UUserWidget* NewWidget = CreateWidget<UUserWidget>(GetWorld(), Def->WidgetClass);
    if (!NewWidget) return false;

    RegisterWidget(NewWidget, Def->Type, Def->Mode, Def->Progression, Def->Priority, Def->WidgetClass->GetFName());
    NewWidget->AddToViewport();
    UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: ToggleByContext -> Shown %s"), *ContextTag.ToString());
    return true;
}

bool UWidgetSubsystem::ShowByContext(FGameplayTag ContextTag)
{
    if (!ContextTag.IsValid()) return false;
    const FName ContextName = ContextTag.GetTagName();

    // If already present, ensure visible and return
    TArray<FWidgetData> Found = GetAllByContext(ContextName);
    if (Found.Num() > 0)
    {
        for (auto& D : Found)
            if (UUserWidget* W = D.Widget.Get())
                if (!W->IsInViewport()) W->AddToViewport();
        return true;
    }

    // Create new
    if (const FWidgetDefinition* Def = WidgetDefinitions.Find(ContextName))
    {
        if (!Def->WidgetClass) return false;
        UUserWidget* NewWidget = CreateWidget<UUserWidget>(GetWorld(), Def->WidgetClass);
        if (!NewWidget) return false;
        RegisterWidget(NewWidget, Def->Type, Def->Mode, Def->Progression, Def->Priority, Def->WidgetClass->GetFName());
        NewWidget->AddToViewport();
        UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: ShowByContext -> Shown %s"), *ContextTag.ToString());
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("WidgetSubsystem: ShowByContext -> No definition for %s"), *ContextTag.ToString());
    return false;
}

bool UWidgetSubsystem::HideByContext(FGameplayTag ContextTag)
{
    if (!ContextTag.IsValid()) return false;
    const FName ContextName = ContextTag.GetTagName();

    TArray<FWidgetData> Found = GetAllByContext(ContextName);
    if (Found.Num() == 0) return false;

    for (auto& D : Found)
    {
        if (UUserWidget* W = D.Widget.Get())
        {
            W->RemoveFromParent();
            UnregisterWidget(W);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("WidgetSubsystem: HideByContext -> Hid %s"), *ContextTag.ToString());
    return true;
}

// Convenience name-overloads
bool UWidgetSubsystem::ToggleByContextName(FName ContextName)
{
    const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(ContextName);
    return ToggleByContext(Tag);
}
bool UWidgetSubsystem::ShowByContextName(FName ContextName)
{
    const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(ContextName);
    return ShowByContext(Tag);
}
bool UWidgetSubsystem::HideByContextName(FName ContextName)
{
    const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(ContextName);
    return HideByContext(Tag);
}


// --------- Maintenance

void UWidgetSubsystem::PruneWidgets()
{
    // Remove dead entries from map
    int32 Removed = 0;
    for (auto It = ActiveWidgets.CreateIterator(); It; ++It)
    {
        const bool bDeadKey = !It.Key().IsValid();
        const bool bDeadVal = !It.Value().Widget.IsValid();
        if (bDeadKey || bDeadVal)
        {
            It.RemoveCurrent();
            ++Removed;
        }
    }
    if (Removed > 0)
    {
        UE_LOG(LogWidgetSubsystem, Log, TEXT("PruneWidgets: removed %d stale entries"), Removed);
    }

    // Clear dead modal/menu slots
    if (!ActiveModal.Widget.IsValid())
    {
        UE_LOG(LogWidgetSubsystem, Log, TEXT("PruneWidgets: ActiveModal invalidated -> clearing & unlocking"));
        ActiveModal = FWidgetData{};
        if (HasLock(NAME_Modal))
        {
            UnlockInput(NAME_Modal);
        }
    }

    if (!ActiveMenu.Widget.IsValid())
    {
        UE_LOG(LogWidgetSubsystem, Log, TEXT("PruneWidgets: ActiveMenu invalidated -> clearing & unpausing/unlocking"));
        ActiveMenu = FWidgetData{};
        SetPausedForMenu(false);
        if (HasLock(NAME_Menu))
        {
            UnlockInput(NAME_Menu);
        }
    }
}


// ---------- Input & Pause

void UWidgetSubsystem::LockInput(FName Reason, bool bShowCursor)
{
    if (!GetWorld()) return;

    const bool bIsFirstWidget = (GInputLockStack.Num() == 0);

    // Prevent duplicate tokens of the same reason
    if (GInputLockStack.ContainsByPredicate([&](const FInputLockToken& T){ return T.Reason == Reason; }))
    {
        UE_LOG(LogWidgetSubsystem, Verbose, TEXT("LockInput: Reason '%s' already locked"), *Reason.ToString());
        return;
    }

    GInputLockStack.Add({ Reason, bShowCursor });

    
    if (APlayerController* Player = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (bIsFirstWidget)
        {
            EnsureUIIMCLoaded();
            if (UIInputMappingContext)
                if (auto* EI = GetEI(GetWorld())) EI->AddMappingContext(UIInputMappingContext, /*Priority=*/1  );
                UE_LOG(LogWidgetSubsystem, Log, TEXT("UI IMC added."));
        }


        UUserWidget* FocusWidget;

        const bool bIsModal = (Reason == NAME_Modal);
        if (bIsModal) 
        {
            FocusWidget =ActiveModal.Widget.Get();
            FInputModeGameAndUI Mode;
            if (FocusWidget) Mode.SetWidgetToFocus(FocusWidget->TakeWidget());

            Player->SetInputMode(Mode);
            Player->SetShowMouseCursor(bShowCursor);
            Player->bEnableClickEvents     = bShowCursor;
            Player->bEnableMouseOverEvents = bShowCursor;

            ModalGameplayBlock(GetWorld());
        }
        else
        {
            const bool bIsMenu = (Reason == NAME_Menu);
            if (bIsMenu)
            {
                FocusWidget =ActiveMenu.Widget.Get();
                FInputModeUIOnly Mode;
                if (FocusWidget) Mode.SetWidgetToFocus(FocusWidget->TakeWidget());

                Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                Player->SetInputMode(Mode);
                Player->SetShowMouseCursor(bShowCursor);
                Player->bEnableClickEvents     = bShowCursor;
                Player->bEnableMouseOverEvents = bShowCursor;
            }
        }
        UE_LOG(LogWidgetSubsystem, Log, TEXT("LockInput: Pushed '%s' (stack size: %d) Cursor:%s"),
            *Reason.ToString(), GInputLockStack.Num(), bShowCursor ? TEXT("On") : TEXT("Off"));
    }
}


void UWidgetSubsystem::UnlockInput(FName Reason)
{
    if (!GetWorld()) return;

    const int32 Index = GInputLockStack.FindLastByPredicate(
        [&](const FInputLockToken& T){ return T.Reason == Reason; });

    if (Index == INDEX_NONE)
    {
        UE_LOG(LogWidgetSubsystem, Warning, TEXT("UnlockInput: Reason '%s' not found (stack size: %d)"),
            *Reason.ToString(), GInputLockStack.Num());
        return;
    }

    GInputLockStack.RemoveAt(Index);

    if (APlayerController* Player = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (GInputLockStack.Num() > 0)
        {
            // Re-apply the new top lock’s desired UI state
            const FInputLockToken& Top = GInputLockStack.Last();

            UUserWidget* FocusWidget = nullptr;

            if (Top.Reason == NAME_Modal)
            {
                FocusWidget = ActiveModal.Widget.Get();
                FInputModeGameAndUI Mode;
                if (FocusWidget)
                {
                    Mode.SetWidgetToFocus(FocusWidget->TakeWidget());
                }
                Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                Player->SetInputMode(Mode);
                Player->SetShowMouseCursor(Top.bShowCursor);
                Player->bEnableClickEvents     = Top.bShowCursor;
                Player->bEnableMouseOverEvents = Top.bShowCursor;
                
                ModalGameplayBlock(GetWorld());
            }
            else if (Top.Reason == NAME_Menu)
            {
                FocusWidget = ActiveMenu.Widget.Get();
                FInputModeUIOnly Mode;
                if (FocusWidget)
                {
                    Mode.SetWidgetToFocus(FocusWidget->TakeWidget());
                }
                Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                Player->SetInputMode(Mode);
                Player->SetShowMouseCursor(Top.bShowCursor);
                Player->bEnableClickEvents     = Top.bShowCursor;
                Player->bEnableMouseOverEvents = Top.bShowCursor;
            }

            UE_LOG(LogWidgetSubsystem, Log, TEXT("UnlockInput: Popped '%s' -> Top='%s' Cursor:%s (stack size: %d)"),
                *Reason.ToString(), *Top.Reason.ToString(),
                Top.bShowCursor ? TEXT("On") : TEXT("Off"), GInputLockStack.Num());
        }
        else
        {
            // No locks left: restore gameplay input (policy: GameOnly, hide cursor)

            if (auto* EI = GetEI(GetWorld())) 
                if (UIInputMappingContext) {
                    EI->RemoveMappingContext(UIInputMappingContext);
                    UE_LOG(LogWidgetSubsystem, Log, TEXT("UI IMC removed."));
                }

            FInputModeGameOnly Mode;
            Player->SetInputMode(Mode);
            Player->SetIgnoreLookInput(false);
            Player->SetIgnoreMoveInput(false);
            Player->SetShowMouseCursor(false);
            Player->bEnableClickEvents     = (false);
            Player->bEnableMouseOverEvents = (false);       
            Player->CurrentMouseCursor = EMouseCursor::Default;

            if (APawn* Pawn = Player->GetPawn())
            {
                Pawn->EnableInput(Player);
            }
            UE_LOG(LogWidgetSubsystem, Log, TEXT("UnlockInput: Popped '%s' -> stack empty, restored GameOnly"),
                *Reason.ToString());
        }
    }
}


void UWidgetSubsystem::SetPausedForMenu(bool bPaused)
{
    if (!GetWorld()) return;

    const bool bResult = UGameplayStatics::SetGamePaused(GetWorld(), bPaused);
    UE_LOG(LogWidgetSubsystem, Log, TEXT("SetPausedForMenu: %s (result: %s)"),
        bPaused ? TEXT("Paused") : TEXT("Unpaused"),
        bResult ? TEXT("Success") : TEXT("NoChange"));
}

void UWidgetSubsystem::EnsureUIIMCLoaded()
{
    if (UIInputMappingContext) return;

    static const TCHAR* Path = TEXT("/Game/Characters/Player/Input/IMC_UI.IMC_UI"); // <- your asset path
    UIInputMappingContext = Cast<UInputMappingContext>(
        StaticLoadObject(UInputMappingContext::StaticClass(), nullptr, Path));

    if (!UIInputMappingContext)
    {
        UE_LOG(LogWidgetSubsystem, Warning, TEXT("Failed to load UI IMC at %s"), Path);
    }
}

// --------- Debug

void UWidgetSubsystem::DumpStateToLog() const
{
    UE_LOG(LogWidgetSubsystem, Log, TEXT("===== WidgetSubsystem Dump ====="));
    UE_LOG(LogWidgetSubsystem, Log, TEXT("Registry: %d entries"), ActiveWidgets.Num());

    int32 Index = 0;
    for (const auto& KV : ActiveWidgets)
    {
        LogEntry(KV.Value, Index++);
    }

    UE_LOG(LogWidgetSubsystem, Log, TEXT("ActiveModal:"));
    if (ActiveModal.Widget.IsValid())
    {
        LogEntry(ActiveModal);
    }
    else
    {
        UE_LOG(LogWidgetSubsystem, Log, TEXT("  None"));
    }

    UE_LOG(LogWidgetSubsystem, Log, TEXT("ActiveMenu:"));
    if (ActiveMenu.Widget.IsValid())
    {
        LogEntry(ActiveMenu);
    }
    else
    {
        UE_LOG(LogWidgetSubsystem, Log, TEXT("  None"));
    }

    UE_LOG(LogWidgetSubsystem, Log, TEXT("===== End Dump ====="));
}


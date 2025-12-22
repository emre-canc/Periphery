#include "Subsystems/ElectricitySubsystem.h"
#include "Subsystems/ActorRegistrySubsystem.h"
#include "GameFramework/Actor.h"

void UElectricitySubsystem::SetCircuitState(FGameplayTag CircuitTag, bool bPowerOn)
{
    if (!CircuitTag.IsValid()) return;

    // --- 1. HIERARCHY CHECK (The Senior Logic) ---
    // If we are trying to turn ON a sub-grid, ensure the Parent Grid is ON.
    if (bPowerOn)
    {
        FGameplayTag ParentGrid = CircuitTag.RequestDirectParent();
        
        // If there is a parent, and that parent is currently OFF
        if (ParentGrid.IsValid() && !IsCircuitOn(ParentGrid))
        {
            UE_LOG(LogTemp, Warning, TEXT("Electricity: Cannot turn ON '%s' because parent grid '%s' is OFF."), 
                *CircuitTag.ToString(), *ParentGrid.ToString());
            return; // Abort
        }
    }

    // --- 2. UPDATE STATE ---
    CircuitStates.Add(CircuitTag, bPowerOn);

    const UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return;
    auto* Registry = GI->GetSubsystem<UActorRegistrySubsystem>();
    if (!Registry) return;


    // Find intersection: "Actors that are Consumers" AND "Actors on this Circuit"
    TArray<AActor*> CircuitItems = Registry->GetActorsWithIntersection(
        FGameplayTag::RequestGameplayTag("Electricity.Consumer"),
        CircuitTag
    );

    // --- 4. NOTIFY ACTORS ---
    for (AActor* Item : CircuitItems)
    {
        if (Item && Item->Implements<UElectricityInterface>())
        {
            if (bPowerOn)
            {
                IElectricityInterface::Execute_PowerOn(Item);
            }
            else
            {
                IElectricityInterface::Execute_PowerOff(Item);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Electricity: Circuit '%s' set to %s. Affected %d actors."), 
        *CircuitTag.ToString(), bPowerOn ? TEXT("ON") : TEXT("OFF"), CircuitItems.Num());
}

bool UElectricitySubsystem::IsCircuitOn(FGameplayTag CircuitTag) const
{
    // Check local map. Default to TRUE if not found (Grid starts active).
    if (const bool* bState = CircuitStates.Find(CircuitTag))
    {
        return *bState;
    }
    return true; 
}

// ---------- QUERIES ----------

TArray<AActor*> UElectricitySubsystem::GetGridActors() const
{
    if (const UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (auto* Registry = GI->GetSubsystem<UActorRegistrySubsystem>())
        {
            return Registry->GetActors(FGameplayTag::RequestGameplayTag("Electricity"));
        }
    }
    return TArray<AActor*>();
}

TArray<AActor*> UElectricitySubsystem::GetFuseBoxActors() const
{
    if (const UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (auto* Registry = GI->GetSubsystem<UActorRegistrySubsystem>())
        {
        return Registry->GetActors(FGameplayTag::RequestGameplayTag("Electricity.Source.Fusebox"));
        }
    }
    return TArray<AActor*>();
}

TArray<AActor*> UElectricitySubsystem::GetLightActors() const
{
    if (const UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (auto* Registry = GI->GetSubsystem<UActorRegistrySubsystem>())
        {
            return Registry->GetActors(FGameplayTag::RequestGameplayTag("Electricity.Consumer.Light"));
        }
    }
    return TArray<AActor*>();
}

TArray<AActor*> UElectricitySubsystem::GetLightsInRoom(FGameplayTag RoomTag) const
{
    if (const UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (auto* Registry = GI->GetSubsystem<UActorRegistrySubsystem>())
        {
            return Registry->GetActorsWithIntersection(
                FGameplayTag::RequestGameplayTag("Electricity.Consumer.Light"),
                RoomTag
            );
        }
    }
    return TArray<AActor*>();
}

// ---------- SAVE / LOAD ----------

void UElectricitySubsystem::RestoreCircuitStates(const TMap<FGameplayTag, bool>& LoadedStates)
{
    CircuitStates = LoadedStates;
    
    // Re-apply states to the world to ensure visuals match data
    for (const auto& Pair : CircuitStates)
    {
        SetCircuitState(Pair.Key, Pair.Value);
    }
}

// ---------- DEBUG ----------

void UElectricitySubsystem::SetCircuitDebug(FName CircuitTagName, bool bOn)
{
    FGameplayTag Tag = FGameplayTag::RequestGameplayTag(CircuitTagName);
    if (Tag.IsValid())
    {
        SetCircuitState(Tag, bOn);
    }
}

void UElectricitySubsystem::KillAllPower()
{
    // Hard shutdown of known main grids
    SetCircuitState(FGameplayTag::RequestGameplayTag("Electricity.Grid.MainStore"), false);
    SetCircuitState(FGameplayTag::RequestGameplayTag("Electricity.Grid.Street"), false);
    UE_LOG(LogTemp, Warning, TEXT("CMD: All Power KILLED"));
}

void UElectricitySubsystem::RestoreAllPower()
{
    // Hard shutdown of known main grids
    SetCircuitState(FGameplayTag::RequestGameplayTag("Electricity.Grid.MainStore"), true);
    SetCircuitState(FGameplayTag::RequestGameplayTag("Electricity.Grid.Street"), true);
    UE_LOG(LogTemp, Warning, TEXT("CMD: All Power RESTORED"));
}

// public:
//     // Getter for the Save System to read data
//     const TMap<FGameplayTag, bool>& GetCircuitStates() const { return CircuitStates; }

//     // Setter for the Save System to restore data
//     void RestoreCircuitStates(const TMap<FGameplayTag, bool>& LoadedStates)
//     {
//         CircuitStates = LoadedStates;
        
//         // CRITICAL: Re-apply the states to the world immediately!
//         for (const auto& Pair : CircuitStates)
//         {
//             SetCircuitState(Pair.Key, Pair.Value);
//         }
//     }
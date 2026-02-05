#include "Subsystems/LevelStateSubsystem.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/DataLayer/DataLayerInstance.h"
#include "Engine/World.h"

// --- Helper: Get the Manager ---

UDataLayerManager* ULevelStateSubsystem::GetDataLayerManager() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // This static helper finds the correct manager for the current world
    return UDataLayerManager::GetDataLayerManager(World);
}

// --- Gameplay Functions ---

void ULevelStateSubsystem::SetDataLayerState(FName LayerName, EDataLayerRuntimeState TargetState)
{
    UDataLayerManager* Manager = GetDataLayerManager();
    if (!Manager) return;

    // Iterate through all layers
    for (const UDataLayerInstance* LayerInstance : Manager->GetDataLayerInstances())
    {
        // Compare Names (Safe generic way)
        if (LayerInstance && LayerInstance->GetDataLayerShortName() == LayerName.ToString())
        {
            if (const UDataLayerAsset* Asset = LayerInstance->GetAsset())
            {
                Manager->SetDataLayerRuntimeState(Asset, TargetState);
            }
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("LevelState: Layer '%s' not found."), *LayerName.ToString());
}

bool ULevelStateSubsystem::IsDataLayerActive(FName LayerName) const
{
    UDataLayerManager* Manager = GetDataLayerManager();
    if (!Manager) return false;

    for (const UDataLayerInstance* LayerInstance : Manager->GetDataLayerInstances())
    {
        if (LayerInstance && LayerInstance->GetDataLayerShortName() == LayerName.ToString())
        {
            // Check if the current state is Activated
            return LayerInstance->GetRuntimeState() == EDataLayerRuntimeState::Activated;
        }
    }

    return false;
}

// --- Save/Load System ---

TArray<FName> ULevelStateSubsystem::GetActiveLayerNames() const
{
    TArray<FName> ActiveLayers;
    UDataLayerManager* Manager = GetDataLayerManager();

    if (!Manager) return ActiveLayers;

    for (const UDataLayerInstance* LayerInstance : Manager->GetDataLayerInstances())
    {
        if (LayerInstance && LayerInstance->GetRuntimeState() == EDataLayerRuntimeState::Activated)
        {
            // Add the name to our list
            ActiveLayers.Add(FName(*LayerInstance->GetDataLayerShortName()));
        }
    }

    return ActiveLayers;
}

void ULevelStateSubsystem::LoadActiveLayerNames(const TArray<FName>& LayerNamesToActivate)
{
    UDataLayerManager* Manager = GetDataLayerManager();
    if (!Manager) return;

    // 1. First, we iterate through ALL layers to reset/update them.
    // This is safer than just turning on the new ones, as it ensures we match the save file exactly.
    for (const UDataLayerInstance* LayerInstance : Manager->GetDataLayerInstances())
    {
        if (!LayerInstance) continue;

        FName CurrentLayerName = FName(*LayerInstance->GetDataLayerShortName());

        // Check if this layer is inside our "To Activate" list
        bool bShouldBeActive = LayerNamesToActivate.Contains(CurrentLayerName);
        EDataLayerRuntimeState TargetState = bShouldBeActive ? EDataLayerRuntimeState::Activated : EDataLayerRuntimeState::Unloaded;

        if (const UDataLayerAsset* Asset = LayerInstance->GetAsset())
        {
            Manager->SetDataLayerRuntimeState(Asset, TargetState);
        }
    }
}
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldPartition/DataLayer/DataLayerInstance.h" 
#include "LevelStateSubsystem.generated.h"

/**
 * Manages the Persistence of World Partition Data Layers.
 * Acts as a wrapper around UWorldPartitionSubsystem to make Saving/Loading easier.
 */
UCLASS()
class INSIDETFV03_API ULevelStateSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // --- Gameplay Functions ---

    /** Turn a Data Layer On or Off by its Label (Name) */
    UFUNCTION(BlueprintCallable, Category="LevelState")
    void SetDataLayerActive(FName LayerName, bool bIsActive);

    /** Check if a layer is currently active */
    UFUNCTION(BlueprintPure, Category="LevelState")
    bool IsDataLayerActive(FName LayerName) const;

    // --- Save/Load System ---

    /** * Returns a list of ALL currently active Data Layer Names.
     * Call this when creating a Save Game object.
     */
    UFUNCTION(BlueprintCallable, Category="LevelState|Save")
    TArray<FName> GetActiveLayerNames() const;

    /** * Takes a list of Names (from a Save File) and forces those layers to activate.
     * Call this when Loading a Game.
     */
    UFUNCTION(BlueprintCallable, Category="LevelState|Save")
    void LoadActiveLayerNames(const TArray<FName>& LayerNamesToActivate);

private:
    // Helper to get the Engine's native Data Layer Manager
    class UDataLayerManager* GetDataLayerManager() const;
};
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Inventory/InventoryStructs.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEquip, AActor*, NewActor, EItemCategory, Category, EHandGripType, NewGrip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnequip);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INSIDETFV03_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnEquip OnEquipEvent;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnUnequip OnUnequipEvent;

    // --- The Core Map ---
    // This matches the type in your UPeripherySaveGame exactly.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
    TMap<FGameplayTag, FInventoryData> InventoryMap;

    // --- API Functions ---

    /** * Finds an item. 
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    FInventoryData FindItemByTag(FGameplayTag ItemTag, bool& bFound, EItemCategory& ItemCategory);

    /** * Reduces item quantity by 1. 
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ConsumeItem(FGameplayTag ItemTag, bool& bConsumed, bool& bDepleted);

    /** * Adds an item. Stacks if exists, creates new if not. 
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddToInventory(FGameplayTag ItemTag, FInventoryData ItemData);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool CanAddToInventory(FGameplayTag ItemTag, FInventoryData ItemData);

    /** * The Brain: Finds the next equippable item and calls PerformEquip.
     * @param Direction +1 for Next, -1 for Previous.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    void CycleEquippedItem(int32 Direction);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool EquipItemByTag(FGameplayTag TagToEquip);

        /** * Input Action: "Holster" (Press H) */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    void HolsterEquippedItem(); 

private:
    // --- INTERNAL MECHANICS (The Muscle) ---

    // Handles the physical spawning and attaching.
    void PerformEquip(const FInventoryData& ItemData, FGameplayTag ItemTag);

    void PerformUnequip();

    void HandleItemDrop(FGameplayTag DroppedTag); // needs improvement

    bool IsEquippable(FGameplayTag CheckTag);

    bool IsOwnerAvailable() const; 

    void PruneCache();

protected:
    // The "Playlist" - Ordered, stable list for cycling
    // This ensures "Item 3" stays "Item 3" until you drop it.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
    TArray<FGameplayTag> InventoryOrder;

    UPROPERTY()
    TMap<FGameplayTag, AActor*> SpawnedActorCache;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|State")
    FGameplayTag LastEquippedTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
    AActor* EquippedActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
    FGameplayTag EquippedActorTag;

};
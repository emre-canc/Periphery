#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Inventory/InventoryStructs.h"
#include "InventoryComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INSIDETFV03_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UInventoryComponent();

    // --- The Core Map ---
    // This matches the type in your UPeripherySaveGame exactly.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<FGameplayTag, FInventoryData> InventoryMap;

    // --- API Functions ---

    /** * Finds an item. 
     * @param bFound True if item exists.
     * @param bKeyItem True if the item is marked as a Key Item.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    FInventoryData FindItemByTag(FGameplayTag ItemTag, bool& bFound, bool& bKeyItem);

    /** * Reduces item quantity by 1. 
     * @param bDepleted True if the item count reached 0 and was removed. 
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool ConsumeItem(FGameplayTag ItemTag);

    /** * Adds an item. Stacks if exists, creates new if not. 
     * @return True if successfully added (e.g. didn't exceed MaxQuantity).
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddToInventory(FGameplayTag ItemTag, FInventoryData ItemData);
};
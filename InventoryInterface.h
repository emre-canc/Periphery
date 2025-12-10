#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Inventory/InventoryStructs.h"
#include "InventoryInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryInterface : public UInterface
{
    GENERATED_BODY()
};

class INSIDETFV03_API IInventoryInterface
{
    GENERATED_BODY()

public:
    // --- API Functions ---

    /** Adds an item to the inventory. Returns success. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
    void AddToInventory(FGameplayTag ItemTag, FInventoryData ItemData, bool& bSuccess);

    /** Checks if an item exists. Returns true if found, and whether it is a Key Item. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
    void FindItemByTag(FGameplayTag ItemTag, FInventoryData& ItemData, bool& bFound, bool& bKeyItem);

    /** Removes one instance of the item. Returns true if consumed. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
    void ConsumeItem(FGameplayTag ItemTag, bool& bConsumed);
};
#include "Inventory/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

FInventoryData UInventoryComponent::FindItemByTag(FGameplayTag ItemTag, bool& bFound, bool& bKeyItem)
{
    // 1. Try to find the item in the map
    if (FInventoryData* FoundItem = InventoryMap.Find(ItemTag))
    {
        bFound = true;
        bKeyItem = FoundItem->bKeyItem;
        return *FoundItem; // Return copy of the data
    }

    // 2. Default fail state
    bFound = false;
    bKeyItem = false;
    return FInventoryData(); // Return empty struct
}

bool UInventoryComponent::ConsumeItem(FGameplayTag ItemTag)
{
    // 1. Find the item
    if (FInventoryData* FoundItem = InventoryMap.Find(ItemTag))
    {
        // 2. Reduce Quantity
        FoundItem->Quantity--;

        // 3. Check for Depletion
        if (FoundItem->Quantity <= 0)
        {
            InventoryMap.Remove(ItemTag);
            return true; // Item is fully depleted/removed
        }
        
        return false; // Item still exists, just reduced
    }

    return false; // Item wasn't there to begin with
}

bool UInventoryComponent::AddToInventory(FGameplayTag ItemTag, FInventoryData InItemData)
{
    // 1. Check if we already have it
    if (FInventoryData* ExistingItem = InventoryMap.Find(ItemTag))
    {
        // A. Check Capacity
        if (ExistingItem->Quantity >= ExistingItem->MaxQuantity)
        {
            return false; // Inventory full for this item
        }

        // B. Add to Stack
        ExistingItem->Quantity++;
        return true;
    }
    else
    {
        // 2. New Item: Ensure quantity is at least 1
        FInventoryData NewItem = InItemData;
        NewItem.Quantity = FMath::Max(1, NewItem.Quantity);

        // 3. Add to Map
        InventoryMap.Add(ItemTag, NewItem);
        return true;
    }
}
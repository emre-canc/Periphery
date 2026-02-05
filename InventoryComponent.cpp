#include "Inventory/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/EquippableInterface.h"
#include "Interfaces/InteractorInterface.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

FInventoryData UInventoryComponent::FindItemByTag(FGameplayTag ItemTag, bool& bFound, EItemCategory& ItemCategory)
{
    // 1. Try to find the item in the map
    if (FInventoryData* FoundItem = InventoryMap.Find(ItemTag))
    {
        bFound = true;
        ItemCategory = FoundItem->ItemCategory;
        return *FoundItem; // Return copy of the data
    }

    // 2. Default fail state
    bFound = false;
    ItemCategory = EItemCategory::None;
    return FInventoryData(); // Return empty struct
}

void UInventoryComponent::ConsumeItem(FGameplayTag ItemTag, bool& bConsumed, bool& bDepleted)
{
    // 1. Find the item
    if (FInventoryData* FoundItem = InventoryMap.Find(ItemTag))
    {
        // 2. Reduce Quantity
        FoundItem->Quantity--;

        // 3. Check for Depletion
        if (FoundItem->Quantity <= 0)
        {
            bConsumed = true;
            bDepleted = true; // Item is fully depleted/removed
        }
        
        bConsumed = true;
        bDepleted = false;
    }

    bConsumed = false;
    bDepleted = false; // Item wasn't there to begin with
}

bool UInventoryComponent::AddToInventory(FGameplayTag ItemTag, FInventoryData InvItemData)
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
        FInventoryData NewItem = InvItemData;
        NewItem.Quantity = FMath::Max(1, NewItem.Quantity);

        
        // 3. Add to Map
        InventoryMap.Add(ItemTag, NewItem);

        if (!InventoryOrder.Contains(ItemTag))
        {
            InventoryOrder.Add(ItemTag);
        }
        if (IsEquippable(ItemTag) && IsOwnerAvailable()) PerformEquip(NewItem, ItemTag);
        return true;
    }
}

bool UInventoryComponent::CanAddToInventory(FGameplayTag ItemTag, FInventoryData InvItemData)
{
    // 1. Check if we already have it
    if (FInventoryData* ExistingItem = InventoryMap.Find(ItemTag))
    {
        // A. Check Capacity
        if (ExistingItem->Quantity >= ExistingItem->MaxQuantity)
        {
            return false; // Inventory full for this item
        }
        return true;
    }
    else
    {
        return true;
    }
}

bool UInventoryComponent::EquipItemByTag(FGameplayTag TagToEquip)
{
    // 1. Safety: Is this a valid, equippable item? (Checks Quantity > 0 & Category)
    if (!IsEquippable(TagToEquip) || !IsOwnerAvailable()) return false;

    // 2. Optimization: Are we already holding this exact item?
    // If yes, you might want to force a "Re-draw" animation, or just do nothing.
    // Currently, calling PerformEquip again will holster and re-draw it, 
    // which is good for "refreshing" the state.
    if (EquippedActorTag == TagToEquip)
    {
        return true; 
    }

    // 3. Execution
    if (FInventoryData* Data = InventoryMap.Find(TagToEquip))
    {
        PerformEquip(*Data, TagToEquip);
        return true;
    }

    return false;
}

void UInventoryComponent::CycleEquippedItem(int32 Direction) // 0 = needs to be holstered
{
    // 1. HARD SAFETY: Empty Inventory or Owner available
    if (InventoryOrder.Num() == 0 || !IsOwnerAvailable()) return;

    int32 StartIndex = 0;
    
    // We force the loop to go Forward (+1) if we are resuming from empty hands,
    // otherwise we respect the player's scroll direction.
    int32 SearchDirection = Direction; 

    // --- PHASE 1: DETERMINE START POINT ---

    if (!EquippedActorTag.IsValid()) // We are HOLSTERED
    {
        // A. Try to Resume Logic (Fast Track)
        if (LastEquippedTag.IsValid())
        {
            bool bIsEquippable = IsEquippable(LastEquippedTag);
            FInventoryData* LastData = InventoryMap.Find(LastEquippedTag);

            if (bIsEquippable)
            {
                PerformEquip(*LastData, LastEquippedTag);
                return; // EXIT EARLY: We successfully resumed.
            }
        }

    // B. Fallback: If memory failed, start scanning from Index 0
        StartIndex = 0;
        SearchDirection = 1; // Always search forward from start
    }
    else // We are EQUIPPED
    {
        // Find where we are now
        int32 CurrentIndex = 0;
        InventoryOrder.Find(EquippedActorTag, CurrentIndex);

        // Start looking at the very next neighbor
        StartIndex = CurrentIndex + Direction;

        SearchDirection = Direction; 
    }

    int32 CurrentCheckIndex = StartIndex;
    int32 ItemsChecked = 0; // Safety counter to prevent infinite loops

    // --- PHASE 2: THE SEARCH LOOP (Skip Passives) ---
    


    // Initial Wrap Fix (In case StartIndex was out of bounds immediately)
    if (CurrentCheckIndex >= InventoryOrder.Num()) CurrentCheckIndex = 0;
    else if (CurrentCheckIndex < 0) CurrentCheckIndex = InventoryOrder.Num() - 1;

    while (ItemsChecked < InventoryOrder.Num())
    {
        FGameplayTag CheckTag = InventoryOrder[CurrentCheckIndex];
        bool bIsEquippable = IsEquippable(CheckTag);

        if (bIsEquippable)
        {
            FInventoryData* CheckData = InventoryMap.Find(CheckTag);

            // SUCCESS: We found a valid item. Equip it.
            PerformEquip(*CheckData, CheckTag);
            return; 
        }

        // FAILURE: Item was Passive/Consumable. Move to next.
        CurrentCheckIndex += SearchDirection;

        // Wrap Around Logic
        if (CurrentCheckIndex >= InventoryOrder.Num()) 
            CurrentCheckIndex = 0;
        else if (CurrentCheckIndex < 0) 
            CurrentCheckIndex = InventoryOrder.Num() - 1;

        ItemsChecked++;
    }
    // --- PHASE 3: TOTAL FAILURE ---
    // If we reach here, we checked every single item in the inventory 
    // and NONE of them were equippable (e.g., Player only has 5 Apples).
    // Do nothing. Stay Holstered.
}

void UInventoryComponent::HolsterEquippedItem()
{
    if (IsOwnerAvailable()) PerformUnequip();
}

void UInventoryComponent::PerformEquip(const FInventoryData& ItemData, FGameplayTag ItemTag)
{
    // Make sure the weapon isn't equipped.
    if (ItemTag == EquippedActorTag) return;

    // Cleanup Old Actor (Calls OnUnequip -> Weapon Hides Itself)
    PerformUnequip();

    AActor* Owner = GetOwner();
    if (!Owner || !ItemData.ItemClass) return;

    // A. Define the pointer OUTSIDE the if/else so we can use it later
    AActor* ActorToEquip = nullptr; 

    // --- STEP 1: TRY TO FIND IT IN CACHE ---
    if (AActor** FoundActor = SpawnedActorCache.Find(ItemTag))
    {
        ActorToEquip = *FoundActor;
        
        // Safety: If it was destroyed (e.g. by a kill command), clear it.
        if (!IsValid(ActorToEquip))
        {
            SpawnedActorCache.Remove(ItemTag);
            ActorToEquip = nullptr; 
        }
    } 
    
    // --- STEP 2: IF NOT FOUND, SPAWN IT ---
    if (!ActorToEquip) 
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = Owner; 
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        ActorToEquip = GetWorld()->SpawnActor<AActor>(ItemData.ItemClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        
        if (!ActorToEquip) return;

        // B. Get Attach Data (Interface)
        FName TargetSocket = FName("Hand_L_Socket");
        FTransform TargetTransform = FTransform::Identity;

        if (ActorToEquip->Implements<UEquippableInterface>())
        {
            FHandAttachData Data = IEquippableInterface::Execute_GetEquipData(ActorToEquip);
            TargetSocket = Data.AttachSocket;
            TargetTransform = Data.RelativeTransform;
        }

        // C. Attach (ONLY HAPPENS ONCE)
        if (ACharacter* CharOwner = Cast<ACharacter>(Owner))
        {
            ActorToEquip->AttachToComponent(
                CharOwner->GetMesh(), 
                FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
                TargetSocket
            );
            ActorToEquip->SetActorRelativeTransform(TargetTransform);
        }
        else
        {
            ActorToEquip->AttachToActor(Owner, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocket);
        }

        // D. Save to Cache
        bool bNeedsCaching = (ItemData.ItemCategory == EItemCategory::Weapon || 
                            ItemData.ItemCategory == EItemCategory::Tool);

        if (bNeedsCaching)
        {
        SpawnedActorCache.Add(ItemTag, ActorToEquip);
        }
    }

    // --- STEP 3: WAKE UP AND SYNC ---
    
    EquippedActor = ActorToEquip;
    EquippedActorTag = ItemTag;
    EquippedActor->SetActorHiddenInGame(false);
    EquippedActor->SetActorEnableCollision(false);

    EHandGripType TargetGrip = EHandGripType::None;

    if (ActorToEquip->Implements<UEquippableInterface>())
    {
        // 1. Get the Grip Type for the Player Animation
        FHandAttachData Data = IEquippableInterface::Execute_GetEquipData(ActorToEquip);
        TargetGrip = Data.GripType;

        // 2. Wake the Gun (Blueprint Logic: Unhide, Enable Tick, etc.)
        IEquippableInterface::Execute_OnEquip(ActorToEquip);
    }

    // 3. Update Player State
    if (OnEquipEvent.IsBound())
    {
        OnEquipEvent.Broadcast(ActorToEquip, ItemData.ItemCategory, TargetGrip);
    }
}

void UInventoryComponent::PerformUnequip()
{
    // 1. Save the memory (The "Bookmark")
    if (EquippedActorTag.IsValid())
    {
        LastEquippedTag = EquippedActorTag;
    }

    // 2. Cleanup Old Actor
    if (EquippedActor)
    {
        if (OnUnequipEvent.IsBound())
        {
            OnUnequipEvent.Broadcast();
        }
        if (EquippedActor->Implements<UEquippableInterface>())
        {
            IEquippableInterface::Execute_OnUnequip(EquippedActor);
        }

        bool bIsCached = SpawnedActorCache.Contains(EquippedActorTag);
        if (bIsCached)
        {
            // KEEP IT: Just hide it.
            EquippedActor->SetActorHiddenInGame(true);
            EquippedActor->SetActorEnableCollision(false);
        }
        else
        {
            // TRASH IT: Destroy completely.
            EquippedActor->Destroy();
        }

        EquippedActor = nullptr;
    }
    EquippedActorTag = FGameplayTag::EmptyTag;
    PruneCache();
}

void UInventoryComponent::HandleItemDrop(FGameplayTag DroppedTag) // needs improvement
{
    // If we just dropped the gun we are currently holding...
    if (DroppedTag == EquippedActorTag)
    {
        // 1. Destroy the visual representation immediately
        PerformUnequip();

        // 2. Reset Tag
        EquippedActorTag = FGameplayTag::EmptyTag;

        // 3. Auto-Switch to next available item
        CycleEquippedItem(1);
    }
}

bool UInventoryComponent::IsEquippable(FGameplayTag CheckTag)
{
    FInventoryData* CheckData = InventoryMap.Find(CheckTag);
    if (CheckData)
    {
        if (CheckData->ItemCategory != EItemCategory::None &&
            CheckData->ItemCategory != EItemCategory::Passive &&
            CheckData->Quantity > 0)   return true;
    }
    return false;
}

bool UInventoryComponent::IsOwnerAvailable() const
{
    AActor* Owner = GetOwner();
    if (!Owner || !Owner -> Implements<UInteractorInterface>()) return false;

    EInteractorStates State = IInteractorInterface::Execute_GetInteractorState(Owner);

    return (State == EInteractorStates::Free || State == EInteractorStates::Equipped);
}


void UInventoryComponent::PruneCache()
{
    // Iterate backwards or use an iterator to safely remove items while looping
    for (auto It = SpawnedActorCache.CreateIterator(); It; ++It)
    {
        AActor* CachedActor = It.Value();

        // If the actor is null, pending kill, or fully destroyed...
        if (!IsValid(CachedActor)) 
        {
            It.RemoveCurrent(); // Safe removal
        }
    }
}




#pragma once

#include "CoreMinimal.h"
#include "InventoryStructs.generated.h"

// 1. ITEM CATEGORY
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	None,

	// LOGIC: Can't be equipped.
    Passive,
    
    // LOGIC: Enable Crosshair, Bind Recoil, Show Ammo UI
    Weapon, 
    
    // LOGIC: Enable Interaction Prompt, Show Battery UI, No Crosshair
    Tool,   
    
    // LOGIC: Passive. Just a mesh in hand. No UI changes.
    KeyItem, 
    
    // LOGIC: One-time use. Consumed immediately (e.g., Pills).
    Consumable 
};


//2. ITEM DATA
USTRUCT(BlueprintType)
struct FInventoryData
{
	GENERATED_BODY()

	/** Class of the item */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Item Class", MakeStructureDefaultValue="None"))
	TObjectPtr<UClass> ItemClass;

	/** Whether the item is needed for game progression */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Item Category"))
	EItemCategory ItemCategory;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Can Drop", MakeStructureDefaultValue="False"))
	bool bCanDrop;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Quantity", MakeStructureDefaultValue="0"))
	int32 Quantity;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Max Quantity", MakeStructureDefaultValue="3"))
	int32 MaxQuantity;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Thumbnail"))
	TObjectPtr<UTexture2D> Thumbnail;

};




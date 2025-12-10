
#pragma once

#include "CoreMinimal.h"
#include "InventoryStructs.generated.h"

/** Please add a struct description */
USTRUCT(BlueprintType)
struct FInventoryData
{
	GENERATED_BODY()

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="ItemClass", MakeStructureDefaultValue="None"))
	TObjectPtr<UClass> ItemClass;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="bKeyItem", MakeStructureDefaultValue="False"))
	bool bKeyItem;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="bCanDrop", MakeStructureDefaultValue="False"))
	bool bCanDrop;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Quantity", MakeStructureDefaultValue="0"))
	int32 Quantity;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="MaxQuantity", MakeStructureDefaultValue="3"))
	int32 MaxQuantity;
};

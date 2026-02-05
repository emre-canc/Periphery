#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Items/InteractableStructs.h"
#include "EquippableInterface.generated.h"


// EquippableInterface.h
UINTERFACE(MinimalAPI)
class UEquippableInterface : public UInterface { GENERATED_BODY() };

class INSIDETFV03_API IEquippableInterface
{
    GENERATED_BODY()
public:
    // Returns true if the item was successfully used
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment|Use")
    bool OnUse(AActor* Interactor); 

    // Ask the item how it should be held
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment|Query")
    FHandAttachData GetEquipData();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment|Equip")
    void OnEquip(); 

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment|Equip")
    void OnUnequip(); 
};
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Items/InteractableStructs.h"
#include "InteractableInterface.generated.h"


UINTERFACE(MinimalAPI, BlueprintType)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class INSIDETFV03_API IInteractableInterface
{
	GENERATED_BODY()

public:
	// --- MAIN ENTRY ---	

	/** Player inputs interact -> Object decides what happens */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	void Interact(AActor* Interactor, EInteractInput InputType);

	// --- OBJECT STATE NOTIFICATIONS ---
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|Carry")
	FHandAttachData GetCarryData();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|Carry")
	void IsThrowable(bool& bIsThrowable, UPrimitiveComponent*& ThrownComponent); 

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|Inspect")
	FInspectData GetInspectData();

	// --- OBJECT STATE SETTERS ---

	// "Hey Item, a specific interaction type just Started (true) or Ended (false)."
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|State")
	void SetItemState(EInteractions InteractionType, bool bActive);

	/** Enable/Disable interaction modes (e.g. Lock the door) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|State")
	void SetInteractModes(EInteractions InteractMode, bool bValue);




};
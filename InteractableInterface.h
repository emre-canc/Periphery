#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Player/InteractionStructs.h"
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
	void Interact(AActor* Interactor, bool bHeldButton, EInteractorStates InteractorState);

	// --- OBJECT STATE NOTIFICATIONS ---

	/** Tell the Object: "You are currently being carried (turn off physics)" */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|Carry")
	void NotifyCarriedActor(bool bIsCarried);

	/** Tell the Object: "You are currently being inspected (spin around)" */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|Inspect")
	void NotifyInspectedActor(bool bIsInspected);

	/** Enable/Disable interaction modes (e.g. Lock the door) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	void SetInteractModes(EInteractions InteractMode, bool bValue);

	// --- DATA GETTERS ---

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|Carry")
	void IsThrowable(bool& bIsThrowable, UPrimitiveComponent*& ThrownComponent);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|Data")
	void GetItemName(FText& ItemName);

};
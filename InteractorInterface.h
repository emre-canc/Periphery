#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Player/InteractorStructs.h"
#include "InteractorInterface.generated.h"



UINTERFACE(MinimalAPI, BlueprintType)
class UInteractorInterface : public UInterface
{
	GENERATED_BODY()
};

class INSIDETFV03_API IInteractorInterface
{
	GENERATED_BODY()

public:
	// --- ACTIONS THE PLAYER DOES ---

	/** Command the Player to attach 'ActorToCarry' to their 'AttachSocket' */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor|Carry")
	void CarryActor(AActor* ActorToCarry);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor|Carry")
	void DropActor(bool bDestroyActor = false);

	/** Command the Player to open their Inspection UI for 'ActorToInspect' */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor|Inspect")
	void InspectActor(AActor* ActorToInspect);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor|State")
	void SetInteractorState(EInteractorStates NewState);

	// --- STATE QUERIES ---

	/** Ask the Player what they are currently doing (Idle, Carrying, etc.) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor|Query")
	EInteractorStates GetInteractorState();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor|Query")
	FGameplayTag GetEquippedItemTag();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor|Query")
	USceneComponent* GetViewAnchor();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor|Query|Trace")
	void InteractorLineTrace(FHitResult& HitResult, bool& ReturnValue); // make this somehow a part of primary action.

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactor|Query|Trace")
	void GetTraceHitResult(FHitResult& HitResult);
};
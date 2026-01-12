#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Animation/AnimMontage.h"
#include "Player/InteractionStructs.h"
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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|Carry")
	void CarryActor(AActor* ActorToCarry, FName AttachSocket,
		UAnimMontage* CarryMontage, EInteractorStates CarryType, bool bKeepRelativeScale);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|Carry")
	void DropActor();

	/** Command the Player to open their Inspection UI for 'ActorToInspect' */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|Inspect")
	void InspectActor(AActor* ActorToInspect, const FText& Name, const FText& Description,
		bool bIsReadable, bool bAutoRead, const FText& NoteText, FTransform InspectTransform);

	//Change interactor state (Busy, Carrying, Inspecting, Etc.)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	void SetInteractorState(EInteractorStates NewState);

	// --- STATE QUERIES ---

	/** Ask the Player what they are currently doing (Idle, Carrying, etc.) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	EInteractorStates GetInteractorState();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact|Data")
	void GetInteractHitResult(FHitResult& HitResult);
};
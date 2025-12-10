
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "LevelStateSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ActorRegistrySubsystem.generated.h"

/**
 * 
 */
UCLASS()
class INSIDETFV03_API UActorRegistrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	// --------- Actor Registry ----------
	UFUNCTION(BlueprintCallable, Category="Registry|Tags")
	void RegisterActorForTag(AActor* Actor, FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="Registry|Tags")
	void UnregisterActorForTag(AActor* Actor, FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Registry|Tags")
	TArray<AActor*> GetActorsForTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category="Registry|Tags")
	void RegisterSequenceForTag(AActor* Actor, FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="Registry|Tags")
	void UnregisterSequenceForTag(AActor* Actor, FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Registry|Tags")
	AActor* GetSequenceForTag(FGameplayTag Tag) const;


	// ---------- Save System ----------
	UFUNCTION(BlueprintCallable, Category="Registry|SaveSystem")
    void RegisterSaveableActor(AActor* Actor, FGuid ActorGuid);

    UFUNCTION(BlueprintCallable, Category="Registry|SaveSystem")
    void UnregisterSaveableActor(FGuid ActorGuid);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Registry|SaveSystem")
    AActor* GetActorByGuid(FGuid ActorGuid) const;

    // Returns all actors that need to be saved 
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Registry|SaveSystem")
    TArray<AActor*> GetAllSaveableActors() const;

	// ---------- Actions ----------
	void SendCommandToActor(FGameplayTag ActorTag, FName CommandName);
    void PlayOrStopSequence(FGameplayTag SequenceTag);

protected:

	//Tags
	void PruneTag(FGameplayTag Tag);

private:

	TMap<FGameplayTag, TSet<TWeakObjectPtr<AActor>>> TagToActors;
	TMap<FGameplayTag, TWeakObjectPtr<AActor>> TagToSequence;

	// The "Phonebook" for saving: Maps ID -> Specific Actor
    TMap<FGuid, TWeakObjectPtr<AActor>> GuidToActorMap;

};

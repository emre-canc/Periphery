
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Subsystems/LevelStateSubsystem.h"
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
	UFUNCTION(BlueprintCallable, Category="Registry|Actors")
	void RegisterActorForTag(AActor* Actor, FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="Registry|Actors")
	void UnregisterActorForTag(AActor* Actor, FGameplayTag Tag);

    // Removes this actor from every tag list it is currently registered to.
    UFUNCTION(BlueprintCallable, Category = "Registry|Actors")
    void RemoveActorFromAllTags(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Registry|Data")
	TArray<AActor*> GetActorsForTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Registry|Data")
	TArray<AActor*> GetActors(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Registry|Data")	
    TArray<AActor*> GetActorsWithIntersection(FGameplayTag TagA, FGameplayTag TagB);

	UFUNCTION(BlueprintCallable, Category = "Registry|Data")
    AActor* FindActor(FGameplayTag Tag) const;
	
	// ---------- Save System ----------
	UFUNCTION(BlueprintCallable, Category="Registry|Save")
    void RegisterSaveableActor(AActor* Actor, FGuid ActorGuid);

    UFUNCTION(BlueprintCallable, Category="Registry|Save")
    void UnregisterSaveableActor(FGuid ActorGuid);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Registry|Save|Data")
    AActor* GetActorByGuid(FGuid ActorGuid) const;

    // Returns all actors that need to be saved 
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Registry|Save|Data")
    TArray<AActor*> GetAllSaveableActors() const;

	const TMap<FGuid, TWeakObjectPtr<AActor>>& GetRegisteredActorsMap() const { return GuidToActorMap; }

	// ---------- Actions ----------
	void SendCommandToActor(FGameplayTag ActorTag, FName CommandName);
    // void PlayOrStopSequence(FGameplayTag SequenceTag);  // use Commands "PlaySequence" instead

protected:

	//Tags
	void PruneTag(FGameplayTag Tag);

private:

	TMap<FGameplayTag, TSet<TWeakObjectPtr<AActor>>> TagToActors;

	// The "Phonebook" for saving: Maps ID -> Specific Actor
    TMap<FGuid, TWeakObjectPtr<AActor>> GuidToActorMap;

};

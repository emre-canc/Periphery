// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h" 
#include "ElectricitySubsystem.generated.h"

/**
 * 
 */
class AActor;


UCLASS()
class INSIDETFV03_API UElectricitySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	private:

    UFUNCTION()
    void OnRegisteredActorEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);
	

	public:
	
	UFUNCTION(BlueprintCallable, Category="Electricity")
    void RegisterToFuseBox(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category="Electricity")
    void RegisterToSystem(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category="Electricity|Local")
	void RegisterToLocalSystem(AActor* Actor, FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="Electricity|Local")
	TArray<AActor*> GetLocalSystemByTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="Electricity")
    void SetStorePowerState(bool PowerState);

	UFUNCTION(BlueprintPure, Category="Electricity")
    bool GetStorePowerState();

	UFUNCTION(BlueprintCallable, Category="Electricity")
    void UnregisterFromAll(AActor* Actor);

	UFUNCTION(BlueprintPure)
	void GetFuseBoxActors(TArray<AActor*>& FuseBoxArray) const; // return filtered strong refs

	UFUNCTION(BlueprintPure)
	void GetSystemActors(TArray<AActor*>& SystemArray) const;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> FuseBoxActors;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> SystemActors;



	private:

    TMap<FGameplayTag, TSet<TWeakObjectPtr<AActor>>> TagToActors;

	bool StorePowerState = true;


};

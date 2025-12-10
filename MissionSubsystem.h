#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MissionStructs.h"
#include "MissionData.h"


#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MissionSubsystem.generated.h"

// ====== Subsystem ======

UCLASS(BlueprintType)
class INSIDETFV03_API UMissionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Delegates

	//On Mission Started
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStarted, FGameplayTag, MissionID);
	UPROPERTY(BlueprintAssignable)
	FOnMissionStarted OnMissionStarted;

	//On Mission Completed
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionCompleted, FGameplayTag, MissionID, bool, bSuccess);
	UPROPERTY(BlueprintAssignable)
	FOnMissionCompleted OnMissionCompleted;

	//On Objective Started
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveStarted, FGameplayTag, MissionID, FGameplayTag, ObjectiveID);
	UPROPERTY(BlueprintAssignable)
	FOnObjectiveStarted OnObjectiveStarted;

	//On Objective Completed
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveCompleted, FGameplayTag, MissionID, FGameplayTag, ObjectiveID, bool, bSuccess);
	UPROPERTY(BlueprintAssignable)
	FOnObjectiveCompleted OnObjectiveCompleted;

	//On Event Broadcast
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionEventBroadcast, FGameplayTag, EventTag);
	UPROPERTY(BlueprintAssignable)
	FOnMissionEventBroadcast OnMissionEventBroadcast;

public:

    UPROPERTY()
    TMap<FGameplayTag, UMissionData*> LoadedMissionAssets;

	UPROPERTY(BlueprintReadOnly, Category="Mission|Runtime")
	TMap<FGameplayTag, FMissionRuntimeState> ActiveMissions;

	UPROPERTY(BlueprintReadOnly, Category="Mission|Runtime")
	TMap<FGameplayTag, FMissionRuntimeState> CompletedMissions;


public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Mission control
	UFUNCTION(BlueprintCallable, Category="Mission")
	void StartMission(FGameplayTag MissionID);

	UFUNCTION(BlueprintCallable, Category="Mission")
	void FinishMission(FGameplayTag MissionID, bool bSuccess);

	UFUNCTION(BlueprintCallable, Category="Mission")
	bool IsMissionActive(FGameplayTag MissionID) const;


	// Objective control
	UFUNCTION(BlueprintCallable, Category="Objective")
	void ActivateObjective(FGameplayTag MissionID, FGameplayTag ObjectiveID);

	UFUNCTION(BlueprintCallable, Category="Objective")
	void CompleteObjective(FGameplayTag MissionID, FGameplayTag ObjectiveID, bool bSuccess);

	UFUNCTION(BlueprintCallable, Category="Objective")
	bool IsObjectiveActive(FGameplayTag MissionID, FGameplayTag ObjectiveID) const;


	// Event bus
	UFUNCTION(BlueprintCallable, Category="Mission|Events")
	void EmitActorEvent(AActor* SourceActor, FGameplayTag EventTag);

	// Save System
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void SaveToGame(UPeripherySaveGame* SaveObject);

	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void LoadFromGame(const UPeripherySaveGame* SaveObject);

protected:

	// ---------- Missions ----------
			
	FMissionRuntimeState* GetActiveMissionRuntime(FGameplayTag MissionID);
	const FMissionRuntimeState* GetActiveMissionRuntime(FGameplayTag MissionID) const;

	// Callback function for Async Mission Loading
	void OnMissionAssetLoaded(FPrimaryAssetId LoadedId, FGameplayTag MissionID);
	// Callback function for Async Objective Loading
	const UMissionData* GetMissionAsset(FGameplayTag MissionID) const;

	// ---------- Objectives ----------

	FObjectiveRuntimeState* GetObjectiveRuntime(FGameplayTag MissionID, FGameplayTag ObjectiveID);
	const FObjectiveRuntimeState* GetObjectiveRuntime(FGameplayTag MissionID, FGameplayTag ObjectiveID) const;

	const UMissionObjective* GetObjectiveFromAsset(FGameplayTag MissionID, FGameplayTag ObjectiveID) const;

	// ---------- Event Bus ----------
	void ConsumeEventForObjective(FGameplayTag MissionID, const UMissionObjective* ObjDef,
		 	FObjectiveRuntimeState& ObjRt, FGameplayTag EventTag, AActor* SourceActor);
	void ActivateNextObjectives(FGameplayTag MissionID, const TArray<FGameplayTag>& NextObjectiveIDs);

	void RunActions(const TArray<TObjectPtr<UMissionAction>>& Actions, AActor* ContextActor); 



};
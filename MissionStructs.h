
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MissionStructs.generated.h"


UENUM(BlueprintType)
enum class EProgressState : uint8
{
	NotStarted,
	InProgress,
	Completed,
	Failed
};

// ====== Runtime structs ======

USTRUCT(BlueprintType)
struct FObjectiveStepRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Runtime")
	FName StepID; 

	UPROPERTY(BlueprintReadWrite, Category="Runtime")
	TMap<FGameplayTag, int32> SeenEventCounts;

	UPROPERTY(BlueprintReadWrite, Category="Runtime")
	TSet<AActor*> CountedSources;

	UPROPERTY(BlueprintReadWrite, Category="Runtime")
	bool bStepCompleted = false;
};

USTRUCT(BlueprintType)
struct FObjectiveRuntimeState
{
    GENERATED_BODY()

    // --- Identity & Status ---
    UPROPERTY(BlueprintReadWrite, SaveGame)
    FGameplayTag ObjectiveID;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    EProgressState ProgressState = EProgressState::NotStarted;

    // --- (Generic Storage) ---
    
    // Counters (Kill counts, Steps completed index, Items collected)
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TMap<FName, int32> IntStorage;

    // Flags (IsTimerRunning, HasVisitedLocation, Step_A_Done)
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TMap<FName, bool> BoolStorage;

    // Analog Data (Time remaining, Distance traveled, Percent complete)
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TMap<FName, float> FloatStorage;

    // History (Unique Actor IDs seen, specific choices made)
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TSet<FString> StringStorage;
};

USTRUCT(BlueprintType)
struct FMissionRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Runtime")
	FGameplayTag MissionID; 

	UPROPERTY(BlueprintReadWrite, Category="Runtime")
	EProgressState MissionState = EProgressState::NotStarted;

	UPROPERTY(BlueprintReadWrite, Category="Runtime")
	TMap<FGameplayTag, FObjectiveRuntimeState> ActiveObjectives;

	UPROPERTY(BlueprintReadWrite, Category="Runtime")
	TSet<FGameplayTag> CompletedObjectiveIDs; 
};

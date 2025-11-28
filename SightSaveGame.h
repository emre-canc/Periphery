// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MissionSubsystem.h"
#include "SightSaveGame.generated.h"


UCLASS()
class INSIDETFV03_API USightSaveGame : public USaveGame
{
	GENERATED_BODY()
	public:

	UPROPERTY(BlueprintReadWrite)
	FName LastCompletedMission;

	// UPROPERTY(BlueprintReadWrite)
	// TArray<FMissionDefinition> SavedMissions;

	// 	UPROPERTY(BlueprintReadWrite)
	// TArray<FMissionDefinition> SavedCompletedMissions;

	// any other game state needed?

};

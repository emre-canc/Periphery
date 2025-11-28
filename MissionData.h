// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MissionObjective.h" 
#include "MissionData.generated.h"

/**
 * 
 */
UCLASS()
class INSIDETFV03_API UMissionData : public UPrimaryDataAsset
{
	GENERATED_BODY()


public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FGameplayTag MissionID;

    UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Objectives")
    TArray<TObjectPtr<UMissionObjective>> ObjectiveArray;


    // Using TSoftObjectPtr points to the specific asset file without forcing it to load into memory immediately.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Next Mission")
    TSoftObjectPtr<UMissionData> NextMissionAsset;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Next Mission")
    FGameplayTag NextMissionID;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;


#if WITH_EDITOR
    // Runs every time something is changed in the Editor
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};

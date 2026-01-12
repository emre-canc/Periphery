// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "Objective_Count.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Count (Do X times)")
class INSIDETFV03_API UObjective_Count : public UMissionObjective
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Rules")
    FGameplayTag TargetEvent;

    UPROPERTY(EditAnywhere, Category = "Rules")
    int32 TargetCount = 1;

    UPROPERTY(EditAnywhere, Category = "Rules")
    bool bRequireUniqueSources = false;

    virtual bool OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const override;
    
    virtual bool IsComplete(const FObjectiveRuntimeState& RuntimeState) const override;
};
   
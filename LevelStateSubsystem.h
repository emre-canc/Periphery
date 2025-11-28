// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LevelStateSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class INSIDETFV03_API ULevelStateSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    /** Apply a phase for a channel, e.g., Channel="Area_Woods01", Phase="PhaseB" */
    UFUNCTION(BlueprintCallable, Category="LevelState")
    void ApplyPhase(FName Channel, FName Phase);

    /** Convenience getter for Blueprints */
    UFUNCTION(BlueprintPure, meta=(WorldContext="WorldContextObject"), Category="LevelState")
    static ULevelStateSubsystem* GetLevel(const UObject* WorldContextObject);

    /** Re-scan the world for a channel (call if you stream in more actors later) */
    UFUNCTION(BlueprintCallable, Category="LevelState")
    void RebuildCache(FName Channel);


    // INSPECTING  **SUBJECT TO CHANGE**

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    AActor* GetInspector() const;

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    AActor* GetInspectedActor() const;

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    bool IsInspecting() const { return bIsInspecting; }
    

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    void SetInspector(AActor* NewInspectorActor);

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    void SetInspectedActor(AActor* NewInspectedActor);

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    void SetIsInspecting(bool bNewIsInspecting); 




private:
    struct FChannelCache
    {
        TMap<FName, TArray<TWeakObjectPtr<AActor>>> PhaseActors; // Phase -> Actors
        FName CurrentPhase;
        bool bBuilt = false;
    };

    TMap<FName, FChannelCache> ChannelCaches;

    void EnsureChannelCacheBuilt(UWorld* World, FName Channel);
    static bool TryParsePhaseFromTag(const FName& Tag, FName Channel, FName& OutPhase);
    static void SetActorPhaseVisibility(AActor* Actor, bool bVisible);

    TWeakObjectPtr<AActor> InspectorActor = nullptr;
    TWeakObjectPtr<AActor> InspectedActor = nullptr;
    bool bIsInspecting = false;
};
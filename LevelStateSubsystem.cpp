// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelStateSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"

static FString MakePrefix(const FName Channel)
{
    FString Result = Channel.ToString() + TEXT("_");
    UE_LOG(LogTemp, VeryVerbose, TEXT("LevelStateSubsystem::MakePrefix -> '%s'"), *Result);
    return Result;
}

void ULevelStateSubsystem::ApplyPhase(FName Channel, FName Phase)
{


    UE_LOG(LogTemp, Log, TEXT("LevelStateSubsystem::ApplyPhase -> Applying phase '%s' on channel '%s'"),
        *Phase.ToString(), *Channel.ToString());

    EnsureChannelCacheBuilt(GetWorld(), Channel);

    FChannelCache* Cache = ChannelCaches.Find(Channel);
    if (!Cache)
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::ApplyPhase -> No cache found for channel '%s'"), *Channel.ToString());
        return;
    }

    int32 TotalActors = 0;
    int32 VisibleActors = 0;

    for (auto& Kvp : Cache->PhaseActors)
    {
        const bool bShow = (Kvp.Key == Phase);
        for (TWeakObjectPtr<AActor>& WeakA : Kvp.Value)
        {
            if (AActor* A = WeakA.Get())
            {
                SetActorPhaseVisibility(A, bShow);
                TotalActors++;
                if (bShow) VisibleActors++;
            }
        }
    }

    Cache->CurrentPhase = Phase;

    UE_LOG(LogTemp, Log, TEXT("LevelStateSubsystem::ApplyPhase -> Phase '%s' applied on channel '%s'. Visible: %d / Total: %d"),
        *Phase.ToString(), *Channel.ToString(), VisibleActors, TotalActors);
}

ULevelStateSubsystem* ULevelStateSubsystem::GetLevel(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::GetLevel -> Null WorldContextObject"));
        return nullptr;
    }

    if (UWorld* World = WorldContextObject->GetWorld())
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("LevelStateSubsystem::GetLevel -> Retrieved subsystem for world '%s'"), *World->GetName());
        return World->GetSubsystem<ULevelStateSubsystem>();
    }

    UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::GetLevel -> Failed to get world from context"));
    return nullptr;
}

void ULevelStateSubsystem::RebuildCache(FName Channel)
{
    if (UWorld* World = GetWorld())
    {
        UE_LOG(LogTemp, Log, TEXT("LevelStateSubsystem::RebuildCache -> Rebuilding channel '%s'"), *Channel.ToString());
        ChannelCaches.Remove(Channel);
        EnsureChannelCacheBuilt(World, Channel);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::RebuildCache -> No valid world"));
    }
}

bool ULevelStateSubsystem::TryParsePhaseFromTag(const FName& Tag, FName Channel, FName& OutPhase)
{
    const FString TagStr = Tag.ToString();
    const FString Prefix = MakePrefix(Channel);
    if (TagStr.StartsWith(Prefix))
    {
        const FString PhaseStr = TagStr.Mid(Prefix.Len());
        if (!PhaseStr.IsEmpty())
        {
            OutPhase = FName(*PhaseStr);
            UE_LOG(LogTemp, VeryVerbose, TEXT("LevelStateSubsystem::TryParsePhaseFromTag -> Parsed '%s' as phase '%s' for channel '%s'"),
                *TagStr, *OutPhase.ToString(), *Channel.ToString());
            return true;
        }
    }

    UE_LOG(LogTemp, VeryVerbose, TEXT("LevelStateSubsystem::TryParsePhaseFromTag -> No match for tag '%s' on channel '%s'"),
        *TagStr, *Channel.ToString());
    return false;
}

void ULevelStateSubsystem::EnsureChannelCacheBuilt(UWorld* World, FName Channel)
{
    FChannelCache& Cache = ChannelCaches.FindOrAdd(Channel);
    if (Cache.bBuilt)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("LevelStateSubsystem::EnsureChannelCacheBuilt -> Cache already built '%s'"),
            *Channel.ToString());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("LevelStateSubsystem::EnsureChannelCacheBuilt -> Building cache for channel '%s'"), *Channel.ToString());
    Cache.PhaseActors.Reset();

    int32 RegisteredActors = 0;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* A = *It;
        if (!IsValid(A)) continue;

        for (const FName& Tag : A->Tags)
        {
            FName Phase;
            if (TryParsePhaseFromTag(Tag, Channel, Phase))
            {
                Cache.PhaseActors.FindOrAdd(Phase).Add(A);
                RegisteredActors++;
                break; // one matching tag per actor is enough
            }
        }
    }

    Cache.bBuilt = true;

    UE_LOG(LogTemp, Log, TEXT("LevelStateSubsystem::EnsureChannelCacheBuilt -> Channel '%s' cached %d actors across %d phases"),
        *Channel.ToString(), RegisteredActors, Cache.PhaseActors.Num());
}

void ULevelStateSubsystem::SetActorPhaseVisibility(AActor* Actor, bool bVisible)
{
    if (!IsValid(Actor))
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::SetActorPhaseVisibility -> Invalid actor pointer"));
        return;
    }

    Actor->SetActorHiddenInGame(!bVisible);
    Actor->SetActorEnableCollision(bVisible);

    TInlineComponentArray<UPrimitiveComponent*> Prims(Actor);
    for (UPrimitiveComponent* Prim : Prims)
    {
        Prim->SetVisibility(bVisible, true);
        Prim->SetCollisionEnabled(bVisible ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    }

    UE_LOG(LogTemp, VeryVerbose, TEXT("LevelStateSubsystem::SetActorPhaseVisibility -> %s actor '%s'"),
        bVisible ? TEXT("Shown") : TEXT("Hidden"), *Actor->GetName());
}



    // INSPECTING  **SUBJECT TO CHANGE**


void ULevelStateSubsystem::SetInspector(AActor* NewInspectorActor)
{
        if (!IsValid(NewInspectorActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::SetInspector -> Invalid Inspector Actor"));
        return;
    }
    InspectorActor = NewInspectorActor;
}

void ULevelStateSubsystem::SetInspectedActor(AActor* NewInspectedActor)
{
        if (!IsValid(NewInspectedActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::SetInspectedActor -> Inspected Actor is not valid"));
        return;
    }
    InspectedActor = NewInspectedActor;
}



AActor* ULevelStateSubsystem::GetInspector() const
{
    if (!InspectorActor.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::GetInspector -> Inspector is not valid"));
        return nullptr;
    }
    return InspectorActor.Get();   
}


AActor* ULevelStateSubsystem::GetInspectedActor() const
{
    if (!InspectedActor.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::GetInspectedActor -> Inspected Actor is not valid"));
        return nullptr;
    }
    return InspectedActor.Get();   
}

  
void ULevelStateSubsystem::SetIsInspecting(bool bNewIsInspecting)
{   
    bIsInspecting = bNewIsInspecting;
    if (!bIsInspecting)
    {
        InspectedActor= nullptr;
    } // resets inspected actor after each inspect
}
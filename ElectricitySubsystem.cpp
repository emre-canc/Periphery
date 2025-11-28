// Fill out your copyright notice in the Description page of Project Settings.


#include "ElectricitySubsystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

void UElectricitySubsystem::RegisterToFuseBox(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        UE_LOG(LogTemp, Warning, TEXT("ElectricitySubsystem: RegisterToFuseBox: Invalid actor"));
        return;
    }

    SystemActors.AddUnique(TWeakObjectPtr<AActor>(Actor));
    FuseBoxActors.AddUnique(TWeakObjectPtr<AActor>(Actor));
    UE_LOG(LogTemp, Log, TEXT("ElectricitySubsystem: RegisterToFuseBox and SystemActors: %s (count=%d)"),
        *Actor->GetName(), FuseBoxActors.Num());
}

void UElectricitySubsystem::RegisterToSystem(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        UE_LOG(LogTemp, Warning, TEXT("ElectricitySubsystem: RegisterToSystem: Invalid actor"));
        return;
    }

    SystemActors.AddUnique(TWeakObjectPtr<AActor>(Actor));
    UE_LOG(LogTemp, Log, TEXT("ElectricitySubsystem: RegisterToSystem: %s (count=%d)"),
        *Actor->GetName(), SystemActors.Num());
}

void UElectricitySubsystem::OnRegisteredActorEndPlay(AActor* Actor, EEndPlayReason::Type /*Reason*/)
{
    const int32 RemovedFuse = FuseBoxActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& ActorPtr) { return ActorPtr.Get() == Actor; });
    const int32 RemovedSys  = SystemActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& ActorPtr) { return ActorPtr.Get() == Actor; });

    if (RemovedFuse || RemovedSys)
    {
        UE_LOG(LogTemp, Log, TEXT("ElectricitySubsystem: ElectricitySubsystem: %s removed (Fuse=%d, System=%d)"),
            *Actor->GetName(), RemovedFuse, RemovedSys);
    }
}


    void UElectricitySubsystem::RegisterToLocalSystem(AActor* Actor, FGameplayTag Tag)
    {
        if (!IsValid(Actor) || !Tag.IsValid()) return;

        TSet<TWeakObjectPtr<AActor>>& SetRef = TagToActors.FindOrAdd(Tag);
        const int32 Before = SetRef.Num();
        SetRef.Add(Actor);
    }


	TArray<AActor*> UElectricitySubsystem::GetLocalSystemByTag(FGameplayTag Tag)
    {
            TArray<AActor*> Out;

    if (!Tag.IsValid()) return Out;

    auto Collect = [&](const FGameplayTag& Key)
    {
        if (const TSet<TWeakObjectPtr<AActor>>* SetPtr = TagToActors.Find(Key))
        {
            for (const TWeakObjectPtr<AActor>& Weak : *SetPtr)
            {
                if (AActor* A = Weak.Get())
                {
                    Out.Add(A);
                }
            }
        }
    };

    Collect(Tag);

    // Optionally prune duplicates (unlikely) and nulls
 
    return Out;
}

void UElectricitySubsystem::UnregisterFromAll(AActor* Actor)
{
    if (!IsValid(Actor)) return;

    FuseBoxActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& ActorPtr){ return ActorPtr.Get() == Actor; });
    SystemActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& ActorPtr){ return ActorPtr.Get() == Actor; });

    for (auto& Pair : TagToActors)
    {
        Pair.Value.Remove(TWeakObjectPtr<AActor>(Actor));
    }

    UE_LOG(LogTemp, Log, TEXT("ElectricitySubsystem: Unregistered %s from all systems"), *Actor->GetName());
}


 void UElectricitySubsystem::GetFuseBoxActors(TArray<AActor*>& FuseBoxArray) const
{
    FuseBoxArray.Reset();
    for (const TWeakObjectPtr<AActor>& Weak : FuseBoxActors)
    {
        if (AActor* A = Weak.Get())
        {
            FuseBoxArray.Add(A);
        }
    }
}


 void UElectricitySubsystem::GetSystemActors(TArray<AActor*>& SystemArray) const
 {
        SystemArray.Reset();
    for (const TWeakObjectPtr<AActor>& Weak : SystemActors)
    {
        if (AActor* A = Weak.Get())
        {
            SystemArray.Add(A);
        }
    }
 }



bool UElectricitySubsystem::GetStorePowerState() {return StorePowerState;}


void UElectricitySubsystem::SetStorePowerState(bool PowerState) 
{
    StorePowerState = PowerState;
}
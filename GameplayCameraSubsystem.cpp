//Periphery-EvEGames

#include "Subsystems/GameplayCameraSubsystem.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogGameplayCameraSubsystem, Log, All);


//Idea: make a FadeTo function where the screen goes dark for a second and moves the camera then lights up at the new camera angle.
//Need to make event dispatchers for: OnBlendComplete, OnBlendStarted

void UGameplayCameraSubsystem::RegisterCamera(FGameplayTag Tag, AActor* Actor)
{
    if (!IsValid(Actor)) return;
    auto& Arr = CamerasByTag.FindOrAdd(Tag);
    Arr.AddUnique(Actor);                     // weak under the hood
    TagToActor.FindOrAdd(Actor) = Tag;        // reverse lookup
}

void UGameplayCameraSubsystem::UnregisterCamera(AActor* Actor)
{
    if (!IsValid(Actor)) return;
    if (FGameplayTag* TagPtr = TagToActor.Find(Actor))
    {
        if (TArray<TWeakObjectPtr<AActor>>* Arr = CamerasByTag.Find(*TagPtr))
        {
            Arr->RemoveAll([Actor](const TWeakObjectPtr<AActor>& P){ return P.Get()==Actor || !P.IsValid(); });
            if (Arr->Num()==0) CamerasByTag.Remove(*TagPtr);
        }
        TagToActor.Remove(Actor);
    }
}



void UGameplayCameraSubsystem::BlendToAndBack(FGameplayTag Tag, float BlendTime, float WaitTime)
{
    if (!GetWorld()) return;

    const auto* Array = CamerasByTag.Find(Tag);
    if (!Array) return;

    AActor* Chosen = nullptr;
    for (const auto& Weak : *Array)
        if (AActor* A = Weak.Get()) { Chosen = A; break; }
    if (!Chosen) return;

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        // Freeze input immediately
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
        if (APawn* P = PC->GetPawn()) P->DisableInput(PC);

        UE_LOG(LogGameplayCameraSubsystem, Log, TEXT("BlendToAndBack: → [%s] (Blend=%.2fs, Hold=%.2fs)"),
            *Chosen->GetName(), BlendTime, WaitTime);

        // Blend to the target camera
        PC->SetViewTargetWithBlend(Chosen, BlendTime);

        // After (blend + hold), start blending back
        const float ReturnStartDelay = FMath::Max(BlendTime + WaitTime, 0.0f);
        FTimerHandle HoldHandle;
        GetWorld()->GetTimerManager().SetTimer(
            HoldHandle,
            [this, PC, BlendTime]()
            {
                if (!PC) return;

                if (APawn* PawnNow = PC->GetPawn())
                {
                    UE_LOG(LogGameplayCameraSubsystem, Log, TEXT("BlendToAndBack: ← Player (Blend=%.2fs)"), BlendTime);
                    PC->SetViewTargetWithBlend(PawnNow, BlendTime);
                }
                else
                {
                    UE_LOG(LogGameplayCameraSubsystem, Warning, TEXT("BlendToAndBack: No pawn to blend back to; skipping view change"));
                }

                // Always re-enable input after return blend
                FTimerHandle ReenableHandle;
                GetWorld()->GetTimerManager().SetTimer(
                    ReenableHandle,
                    [PC]()
                    {
                        if (!PC) return;
                        PC->SetIgnoreMoveInput(false);
                        PC->SetIgnoreLookInput(false);
                        if (APawn* P = PC->GetPawn()) P->EnableInput(PC);
                        UE_LOG(LogGameplayCameraSubsystem, Log, TEXT("BlendToAndBack: Input re-enabled"));
                    },
                    FMath::Max(BlendTime, 0.0f),
                    false
                );
            },
            ReturnStartDelay,
            false
        );

        UE_LOG(LogGameplayCameraSubsystem, Verbose, TEXT("BlendToAndBack: Input frozen"));
    }
}

void UGameplayCameraSubsystem::BlendTo(FGameplayTag Tag, float BlendTime)
{
    const auto* Array = CamerasByTag.Find(Tag);
    if (!Array) return;

    AActor* Chosen = nullptr;

    // Policy 1: first valid
    for (const auto& Weak : *Array) { if (AActor* A = Weak.Get()) { Chosen = A; break; } }

    // (Alt) Policy 2: nearest to player, or highest priority, etc.

    if (Chosen)
    {
        if (auto* PC = GetWorld()->GetFirstPlayerController())
        {
            PC->SetViewTargetWithBlend(Chosen, BlendTime);
        }
            
    }
}

void UGameplayCameraSubsystem::BlendToPlayer(float BlendTime)
{
    if (!GetWorld()) return;

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, PC, BlendTime,PlayerPawn]()
                {
                    PC->SetViewTargetWithBlend(PlayerPawn, BlendTime);
                }, 
                BlendTime, false
            ); // waits blend time + extra second
        }
            PC->SetIgnoreMoveInput(false);
            PC->SetIgnoreLookInput(false);
            if (APawn* PPawn = PC->GetPawn()) PPawn->EnableInput(PC);
    }
}

TArray<AActor*> UGameplayCameraSubsystem::FindCameras(FGameplayTag Tag) const
{
    TArray<AActor*> Result;
    if (const TArray<TWeakObjectPtr<AActor>>* Array = CamerasByTag.Find(Tag))
    {
        for (const auto& Weak : *Array)
        {
            if (AActor* Cam = Weak.Get())
            {
                Result.Add(Cam);
            }
        }
    }
    return Result;
}
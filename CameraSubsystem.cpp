#include "Subsystems/CameraSubsystem.h"
#include "Subsystems/ActorRegistrySubsystem.h" 
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraSubsystem, Log, All);

// =========================================================
// ACTION: BLEND TO AND BACK (e.g. Security Camera check)
// =========================================================
void UCameraSubsystem::BlendToAndBack(FGameplayTag Tag, float BlendTime, float WaitTime)
{
    if (!GetWorld()) return;

    // 1. Find Camera via Registry
    TArray<AActor*> Cameras = GetCameras(Tag);
    if (Cameras.IsEmpty()) 
    {
        UE_LOG(LogCameraSubsystem, Warning, TEXT("BlendToAndBack: No camera found for tag %s"), *Tag.ToString());
        return; 
    }
    AActor* Chosen = Cameras[0];

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        // 2. Freeze Input
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
        if (APawn* P = PC->GetPawn()) P->DisableInput(PC);

        UE_LOG(LogCameraSubsystem, Log, TEXT("BlendToAndBack: → [%s] (Blend=%.2fs, Hold=%.2fs)"),
            *Chosen->GetName(), BlendTime, WaitTime);

        // 3. Blend TO Camera
        PC->SetViewTargetWithBlend(Chosen, BlendTime);

        // 4. Cancel any previous return timer so we don't snap back early
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CameraWork);

        // 5. Schedule Return
        const float ReturnStartDelay = FMath::Max(BlendTime + WaitTime, 0.0f);
        
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle_CameraWork, // Store this handle!
            [this, PC, BlendTime]()
            {
                if (!PC) return;

                // Return to Player Pawn
                if (APawn* PawnNow = PC->GetPawn())
                {
                    UE_LOG(LogCameraSubsystem, Log, TEXT("BlendToAndBack: ← Player (Blend=%.2fs)"), BlendTime);
                    PC->SetViewTargetWithBlend(PawnNow, BlendTime);
                }

                // Re-enable Input AFTER the blend finishes
                FTimerHandle ReenableHandle;
                GetWorld()->GetTimerManager().SetTimer(
                    ReenableHandle,
                    [PC]()
                    {
                        if (!IsValid(PC)) return;
                        PC->SetIgnoreMoveInput(false);
                        PC->SetIgnoreLookInput(false);
                        if (APawn* P = PC->GetPawn()) P->EnableInput(PC);
                        UE_LOG(LogCameraSubsystem, Log, TEXT("BlendToAndBack: Input re-enabled"));
                    },
                    FMath::Max(BlendTime, 0.0f),
                    false
                );
            },
            ReturnStartDelay,
            false
        );
    }
}

// =========================================================
// ACTION: BLEND TO CAMERA (One Way)
// =========================================================
void UCameraSubsystem::BlendToCamera(FGameplayTag Tag, float BlendTime)
{
    TArray<AActor*> Cameras = GetCameras(Tag);
    if (Cameras.IsEmpty()) return;

    // Policy: Just grab the first one found
    AActor* Chosen = Cameras[0];

    if (Chosen)
    {
        if (auto* PC = GetWorld()->GetFirstPlayerController())
        {
            // Clear any pending "Back" timers so we stay here
            GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CameraWork);
            
            PC->SetViewTargetWithBlend(Chosen, BlendTime);
        }
    }
}

// =========================================================
// ACTION: BLEND TO PLAYER
// =========================================================
void UCameraSubsystem::BlendToPlayer(float BlendTime)
{
    if (!GetWorld()) return;

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            // Clear any pending timers
            GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CameraWork);

            // 1. Start Blending NOW
            PC->SetViewTargetWithBlend(PlayerPawn, BlendTime);
            
            // 2. Enable Input LATER
            FTimerHandle Handle;
            GetWorld()->GetTimerManager().SetTimer(Handle, [PC]()
            {
                if (IsValid(PC))
                {
                    PC->SetIgnoreMoveInput(false);
                    PC->SetIgnoreLookInput(false);
                    if (APawn* P = PC->GetPawn()) P->EnableInput(PC);
                }
            }, 
            BlendTime, false);
        }
    }
}

// =========================================================
// ACTION: FADE TO CAMERA (Cinematic transition)
// =========================================================
void UCameraSubsystem::FadeToCamera(FGameplayTag Tag, float FadeTime, float WaitTime)
{
    if (!GetWorld()) return;

    TArray<AActor*> Cameras = GetCameras(Tag);
    if (Cameras.IsEmpty()) return;
    AActor* NewTarget = Cameras[0];

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager) return;

    // Clear conflicting timers
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CameraWork);

    // 1. Start Fade OUT (Transparent -> Black)
    PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, FadeTime, FLinearColor::Black, false, true);

    // 2. Schedule the Cut + Fade In
    TWeakObjectPtr<APlayerController> WeakPC(PC);
    TWeakObjectPtr<AActor> WeakTarget(NewTarget);

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_CameraWork, [WeakPC, WeakTarget, FadeTime, WaitTime]()
    {
        if (APlayerController* AlivePC = WeakPC.Get())
        {
            // A. Snap to new camera (Instant, because screen is black)
            if (AActor* AliveTarget = WeakTarget.Get())
            {
                AlivePC->SetViewTarget(AliveTarget);
            }

            // B. Wait 'WaitTime', then Fade IN
            FTimerHandle FadeInHandle;
            AlivePC->GetWorld()->GetTimerManager().SetTimer(FadeInHandle, [WeakPC, FadeTime]()
            {
                if (APlayerController* FinalPC = WeakPC.Get())
                {
                    if (FinalPC->PlayerCameraManager)
                    {
                        // Fade IN (Black -> Transparent)
                        FinalPC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, FadeTime, FLinearColor::Black, false, false);
                    }
                }
            }, WaitTime, false);
        }
    }, FadeTime, false);
}

// =========================================================
// ACTION: CUT (Instant)
// =========================================================
bool UCameraSubsystem::CutToCamera(FGameplayTag Tag)
{
    TArray<AActor*> Cameras = GetCameras(Tag);
    if (Cameras.IsEmpty()) return false;

    AActor* Chosen = Cameras[0];

    if (Chosen)
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CameraWork);
            PC->SetViewTarget(Chosen); // No BlendTime = Instant Cut
            return true;
        }
    }
    return false;
}

// =========================================================
// QUERIES (Bridge to Registry)
// =========================================================

TArray<AActor*> UCameraSubsystem::GetCamerasByTag(FGameplayTag Tag) const
{
    if (const UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (auto* Registry = GI->GetSubsystem<UActorRegistrySubsystem>())
        {
        // Use FindActorsForTag for exact matches (fastest).
        // If you want hierarchy (e.g. "Camera.Hallway" finding "Camera.Hallway.01"), use Registry->FindActors(Tag).
        return Registry->GetActorsForTag(Tag);
        }
    }
    
    return TArray<AActor*>();
}

TArray<AActor*> UCameraSubsystem::GetCameras(FGameplayTag Tag) const
{
    if (const UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (auto* Registry = GI->GetSubsystem<UActorRegistrySubsystem>())
        {
        // Use FindActors() (The Hierarchy Search)
        // If you search for "Camera", this will return "Camera.Hallway", "Camera.Fusebox", etc.
        return Registry->GetActors(Tag);
        }
    }
    
    return TArray<AActor*>();
}

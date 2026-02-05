#include "Core/PeripheryGameMode.h"
#include "Core/PeripheryGameInstance.h"

#include "Subsystems/WidgetSubsystem.h"
#include "Subsystems/CameraSubsystem.h" 
#include "Subsystems/MissionSubsystem.h"
#include "Kismet/GameplayStatics.h"

void APeripheryGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (bTestMission)
    {
        UPeripheryGameInstance* GI = Cast<UPeripheryGameInstance>(GetGameInstance());
        if (GI) 
        {
            if (UMissionSubsystem* MissionSys = GI->GetSubsystem<UMissionSubsystem>())
            {
                // Wipe old state to be safe, then start mission
                MissionSys->ResetSystem(); 
                MissionSys->StartMission(TestMissionTag);
            }
        }
        return;
    }

    if (!bShowMenu) return;
    UPeripheryGameInstance* GI = Cast<UPeripheryGameInstance>(GetGameInstance());
    APlayerController* PC = GetWorld()->GetFirstPlayerController();

    // 1. LOAD GAME FLOW (Skip Menu)
    if (GI && GI->bLoadingSave)
    {
        UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: Loading Save Flow Triggered"));
        if (PC)
        {
            PC->SetInputMode(FInputModeGameOnly());
            PC->SetShowMouseCursor(false);
        }
        GI->bLoadingSave = false;
    }
    // 2. LIVE MENU FLOW (Show Menu)
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: Menu Flow Triggered"));

        // --- DEBUGGING THE CAMERA CUT ---
        if (UCameraSubsystem* CamSys = GetWorld()->GetSubsystem<UCameraSubsystem>())
        {
            // Log what tag we are looking for
            UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: Attempting Cut to Tag: %s"), *MenuCameraTag.ToString());

            // Attempt the cut
            bool bSuccess = CamSys->CutToCamera(MenuCameraTag);

            if (bSuccess)
            {
                UE_LOG(LogTemp, Log, TEXT("GAMEMODE: Camera Cut SUCCESS."));
            }
            else
            {
                // IF THIS PRINTS, IT MEANS THE REGISTRY IS EMPTY (Race Condition)
                UE_LOG(LogTemp, Error, TEXT("GAMEMODE: Camera Cut FAILED. No actors found for tag! (Are they registered yet?)"));
                
                // --- THE FIX: RETRY AFTER A DELAY ---
                // We wait 0.1 seconds to let the Camera Actor finish its BeginPlay registration.
                FTimerHandle RetryHandle;
                GetWorldTimerManager().SetTimer(RetryHandle, [this, CamSys]()
                {
                    CamSys->CutToCamera(MenuCameraTag);
                    UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: Retrying Camera Cut..."));
                }, 0.1f, false);
            }
        }
        else
        {
             UE_LOG(LogTemp, Error, TEXT("GAMEMODE: Camera Subsystem Missing!"));
        }
        // --------------------------------

        // Show the Main Menu Widget
        if (UWidgetSubsystem* WS = GI->GetSubsystem<UWidgetSubsystem>())
        {
            WS->OpenMenu(MainMenuClass);
        }

        if (PC)
        {
            PC->SetShowMouseCursor(true);
            PC->SetInputMode(FInputModeUIOnly());
        }
    }
}

void APeripheryGameMode::StartSessionFromMenu()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    UPeripheryGameInstance* GI = Cast<UPeripheryGameInstance>(GetGameInstance());
    if (!GI) return;

    // 1. CAMERA TRANSITION (Fade to Black -> Switch to Player -> Fade In)
    if (UCameraSubsystem* CamSys = GetWorld()->GetSubsystem<UCameraSubsystem>())
    {
        // 1.5s Fade, 0.5s Hold Black
        CamSys->FadeToCamera(PlayerCameraTag, 1.5f, 0.5f);
    }

    // 2. CLOSE UI
    if (UWidgetSubsystem* WS = GI->GetSubsystem<UWidgetSubsystem>())
    {
        WS->CloseMenu(); 
    }

    // 3. INPUT MODE
    PC->SetShowMouseCursor(false);
    PC->SetInputMode(FInputModeGameOnly());

    // 4. START GAMEPLAY LOGIC
    if (UMissionSubsystem* MissionSys = GI->GetSubsystem<UMissionSubsystem>())
    {
        // Wipe old state to be safe, then start Prologue
        MissionSys->ResetSystem(); 
        MissionSys->StartMission(PrologueMissionTag);
    }
}

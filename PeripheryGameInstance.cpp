// #include "PeripheryGameInstance.h"
// #include "Kismet/GameplayStatics.h"
// #include "PeripherySaveGame.h"

// // --- Subsystems ---
// #include "Missions/MissionSubsystem.h"
// #include "LevelStateSubsystem.h" 

// bool UPeripheryGameInstance::SaveGame(FString SlotName)
// {
//     UE_LOG(LogTemp, Log, TEXT("GameInstance: Attempting to Save to Slot: %s"), *SlotName);

//     // 1. Create the Save Object instance in memory
//     UPeripherySaveGame* SaveObj = Cast<UPeripherySaveGame>(
//         UGameplayStatics::CreateSaveGameObject(UPeripherySaveGame::StaticClass())
//     );

//     if (!SaveObj)
//     {
//         UE_LOG(LogTemp, Error, TEXT("GameInstance: Failed to create SaveGame Object!"));
//         return;
//     }

//     // 2. GATHER DATA (Ask systems to fill the object)

//     // Mission Data
//     if (UMissionSubsystem* MissionSys = GetSubsystem<UMissionSubsystem>())
//     {
//         MissionSys->SaveToGame(SaveObj);
//     }

//     // Level/World Data (Data Layers)
//     if (ULevelStateSubsystem* LevelSys = GetSubsystem<ULevelStateSubsystem>())
//     {
//         // LevelSys->SaveDataLayers(SaveObj); // Implement this in LevelStateSubsystem later
//     }

//     // Player Data (Transform, Inventory)
//     // We grab the player pawn from the world to get their location/inventory
//     if (APawn* PlayerPawn = GetFirstLocalPlayerController()->GetPawn())
//     {
//         SaveObj->PlayerTransform = PlayerPawn->GetActorTransform();
        
//         // If your player implements an Interface or has a public Inventory variable:
//         // SaveObj->PlayerInventory = PlayerPawn->Inventory; 
//     }

//     // 3. WRITE TO DISK
//     bool bSaved = UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, 0);
    
//     if (bSaved)
//     {
//         UE_LOG(LogTemp, Log, TEXT("GameInstance: Successfully wrote save file to disk."));
//     }
//     else
//     {
//         UE_LOG(LogTemp, Error, TEXT("GameInstance: Failed to write save file!"));
//     }
// }

// void UPeripheryGameInstance::LoadGame(FString SlotName)
// {
//     UE_LOG(LogTemp, Log, TEXT("GameInstance: Attempting to Load Slot: %s"), *SlotName);

//     // 1. Verify file exists
//     if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
//     {
//         UE_LOG(LogTemp, Warning, TEXT("GameInstance: Load Failed. Slot %s does not exist."), *SlotName);
//         return;
//     }

//     // 2. READ FROM DISK (Deserialize)
//     USaveGame* RawLoad = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
//     UPeripherySaveGame* LoadObj = Cast<UPeripherySaveGame>(RawLoad);

//     if (LoadObj)
//     {
//         // 3. DISTRIBUTE DATA
//         HandleSaveGameLoaded(LoadObj);
//     }
//     else
//     {
//         UE_LOG(LogTemp, Error, TEXT("GameInstance: Failed to Cast loaded object to UPeripherySaveGame."));
//     }
// }

// void UPeripheryGameInstance::HandleSaveGameLoaded(UPeripherySaveGame* LoadedData)
// {
//     // A. Mission Subsystem
//     if (UMissionSubsystem* MissionSys = GetSubsystem<UMissionSubsystem>())
//     {
//         MissionSys->LoadFromGame(LoadedData);
//     }

//     // B. Level State (Data Layers)
//     if (ULevelStateSubsystem* LevelSys = GetSubsystem<ULevelStateSubsystem>())
//     {
//         // LevelSys->LoadDataLayers(LoadedData); // Implement later
//     }

//     // C. Player Data
//     // Note: If you are loading into a *new* level, the Player Pawn might not exist yet.
//     // If you are just reloading state in the *current* level, this works fine.
//     if (APawn* PlayerPawn = GetFirstLocalPlayerController()->GetPawn())
//     {
//         PlayerPawn->SetActorTransform(LoadedData->PlayerTransform, false, nullptr, ETeleportType::TeleportPhysics);
        
//         // Restore Inventory here if needed
//     }

//     UE_LOG(LogTemp, Log, TEXT("GameInstance: Save Data Distributed to Subsystems."));
// }
#include "Core/PeripheryGameInstance.h"
#include "Core/PeripherySaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

// --- Inventory ---
#include "Inventory/InventoryComponent.h"
// --- Subsystems ---
#include "Subsystems/LevelStateSubsystem.h"
#include "Subsystems/ActorRegistrySubsystem.h"
#include "Subsystems/MissionSubsystem.h"
#include "Interfaces/SaveableInterface.h"

bool UPeripheryGameInstance::SaveGame(FString SlotName)
{
    UE_LOG(LogTemp, Log, TEXT("GameInstance: Starting Save to Slot: %s"), *SlotName);

    // 1. Create the Save Object
    UPeripherySaveGame* SaveObj = Cast<UPeripherySaveGame>(
        UGameplayStatics::CreateSaveGameObject(UPeripherySaveGame::StaticClass())
    );

    if (!SaveObj)
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance: Failed to create SaveGame Object!"));
        return false;
    }

    UWorld* World = GetWorld();
    if (!World) return false;

    // --------------------------------------------------------
    // 2. GATHER SUBSYSTEM DATA
    // --------------------------------------------------------

    // A. Missions
    if (UMissionSubsystem* MissionSys = GetSubsystem<UMissionSubsystem>())
    {
        MissionSys->SaveToGame(SaveObj);
    }

    // B. Level State (Data Layers)
    if (ULevelStateSubsystem* LevelSys = GetWorld()->GetSubsystem<ULevelStateSubsystem>())
    {
        SaveObj->ActiveDataLayers = LevelSys->GetActiveLayerNames();
    }

    // C. Actors (The Heavy Lifting)
    if (UActorRegistrySubsystem* ActorSys = GetSubsystem<UActorRegistrySubsystem>())
    {
        // We iterate the map so we have the GUID keys ready to go
        const auto& RegistryMap = ActorSys->GetRegisteredActorsMap();

        for (const auto& Pair : RegistryMap)
        {
            FGuid ActorID = Pair.Key;
            AActor* Actor = Pair.Value.Get();

            // Check validity and Interface implementation
            if (IsValid(Actor) && Actor->Implements<USaveableInterface>())
            {
                // Call the Interface to get the data packet
                FActorRecord Record = ISaveableInterface::Execute_GetSaveData(Actor);
                
                // Save Class for potential respawning
                Record.ActorClass = Actor->GetClass();
                Record.Transform = Actor->GetActorTransform();

                // Store in Save Object
                SaveObj->SavedActors.Add(ActorID, Record);
            }
        }
    }

    // --------------------------------------------------------
    // 3. GATHER PLAYER DATA
    // --------------------------------------------------------
    
    if (APlayerController* PC = GetFirstLocalPlayerController())
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            // Assuming your Character has this component accessible
            if (UInventoryComponent* InvComp = Pawn->FindComponentByClass<UInventoryComponent>())
            {
                SaveObj->PlayerInventory = InvComp->InventoryMap;
            }
        }
    }

    // --------------------------------------------------------
    // 4. WRITE TO DISK
    // --------------------------------------------------------
    
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, 0);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("GameInstance: Save Complete. Saved %d Actors."), SaveObj->SavedActors.Num());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance: Failed to write to disk!"));
    }

    return bSuccess;
}

bool UPeripheryGameInstance::LoadGame(FString SlotName)
{
    UE_LOG(LogTemp, Log, TEXT("GameInstance: Loading Slot: %s"), *SlotName);

    // 1. Check existence
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogTemp, Warning, TEXT("GameInstance: Load failed. File does not exist."));
        return false;
    }

    // 2. Load from Disk
    UPeripherySaveGame* LoadedObj = Cast<UPeripherySaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (!LoadedObj)
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance: Load failed. Could not cast SaveObject."));
        return false;
    }

    // 3. Distribute Data
    HandleSaveGameLoaded(LoadedObj);
    return true;
}

void UPeripheryGameInstance::HandleSaveGameLoaded(UPeripherySaveGame* LoadedData)
{
    if (!LoadedData) return;

    // --------------------------------------------------------
    // 1. RESTORE WORLD STATE 
    // --------------------------------------------------------
    if (ULevelStateSubsystem* LevelSys = GetWorld()->GetSubsystem<ULevelStateSubsystem>())
    {
        LevelSys->LoadActiveLayerNames(LoadedData->ActiveDataLayers);
    }

    // --------------------------------------------------------
    // 2. RESTORE MISSIONS
    // --------------------------------------------------------
    if (UMissionSubsystem* MissionSys = GetSubsystem<UMissionSubsystem>())
    {
        MissionSys->LoadFromGame(LoadedData);
    }

    // --------------------------------------------------------
    // 3. RESTORE ACTORS
    // --------------------------------------------------------
    if (UActorRegistrySubsystem* ActorSys = GetSubsystem<UActorRegistrySubsystem>())
    {
        // Iterate through all SAVED records
        for (const auto& Pair : LoadedData->SavedActors)
        {
            FGuid ActorID = Pair.Key;
            FActorRecord Record = Pair.Value;

            // 1. Try to find the existing actor
            AActor* FoundActor = ActorSys->GetActorByGuid(ActorID);

            // 2. If found, update it
            if (IsValid(FoundActor) && FoundActor->Implements<USaveableInterface>())
            {
                // Optional: Restore Transform if it moved (e.g. Physics props)
                // FoundActor->SetActorTransform(Record.Transform);

                // Call Interface to unpack data
                ISaveableInterface::Execute_LoadSaveData(FoundActor, Record);
            }
            // 3. If NOT found, this might be a deleted/spawned actor?
            else
            {
                // Logic for respawning destroyed actors goes here.
                // You would use Record.ActorClass and Record.Transform to SpawnActor,
                // then call LoadSaveData on the new instance.
            }
        }
    }

    // --------------------------------------------------------
    // 4. RESTORE PLAYER
    // --------------------------------------------------------
    if (APlayerController* PC = GetFirstLocalPlayerController())
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            // Teleport Player
            Pawn->SetActorTransform(LoadedData->PlayerTransform, false, nullptr, ETeleportType::TeleportPhysics);
        
            if (UInventoryComponent* InvComp = Pawn->FindComponentByClass<UInventoryComponent>())
            {
                // Direct overwrite works because the types (TMap<Tag, Data>) are identical
                InvComp->InventoryMap = LoadedData->PlayerInventory;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("GameInstance: Load Complete. World State Restored."));
}
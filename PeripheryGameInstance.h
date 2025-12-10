// #pragma once

// #include "CoreMinimal.h"
// #include "Engine/GameInstance.h"
// #include "PeripheryGameInstance.generated.h"

// class UPeripherySaveGame;

// UCLASS()
// class INSIDETFV03_API UPeripheryGameInstance : public UGameInstance
// {
//     GENERATED_BODY()

// public:

//     // Gathers data from all subsystems and writes to disk
//     UFUNCTION(BlueprintCallable, Category = "SaveSystem")
//     bool SaveGame(FString SlotName);

//     // Reads from disk and pushes data into subsystems
//     UFUNCTION(BlueprintCallable, Category = "SaveSystem")
//     bool LoadGame(FString SlotName);

// protected:
//     // Internal helper to distribute the loaded data object to subsystems
//     void HandleSaveGameLoaded(UPeripherySaveGame* LoadedData);
// };
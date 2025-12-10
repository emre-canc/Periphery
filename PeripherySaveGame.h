
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "Missions/MissionStructs.h"
#include "Inventory/InventoryStructs.h"
#include "PeripherySaveGame.generated.h"


USTRUCT(BlueprintType)
struct FActorRecord
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
    UClass* ActorClass; 

    UPROPERTY(VisibleAnywhere)
    FTransform Transform; // Position/Rotation

    UPROPERTY(VisibleAnywhere)
    TArray<uint8> ByteData; // The binary of specific variables (bIsOn, AmmoCount, etc.)
};

UCLASS()
class INSIDETFV03_API UPeripherySaveGame : public USaveGame
{
	GENERATED_BODY()
	public:

	// --- Mission Data ---
    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    TMap<FGameplayTag, FMissionRuntimeState> ActiveMissions;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    TMap<FGameplayTag, FMissionRuntimeState> CompletedMissions;

    // --- Player Data ---
    UPROPERTY(VisibleAnywhere, Category = "SaveData|Player")
	TMap<FGameplayTag, FInventoryData> PlayerInventory;

    UPROPERTY(VisibleAnywhere, Category = "SaveData|Player")
    FTransform PlayerTransform;

    // --- World State ---
    UPROPERTY(VisibleAnywhere, Category = "SaveData|World")
    TArray<FName> ActiveDataLayers; 

    // --- Actor Registry ---
    // Maps a specific Object ID (GUID)
    UPROPERTY(VisibleAnywhere, Category = "SaveData|Actors")
    TMap<FGuid, FActorRecord> SavedActors;

};

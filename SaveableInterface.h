#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/PeripherySaveGame.h"
#include "SaveableInterface.generated.h"


UINTERFACE(MinimalAPI)
class USaveableInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Implement this Interface on any Actor (Blueprint or C++) that needs to save state.
 */
class INSIDETFV03_API ISaveableInterface
{
    GENERATED_BODY()

public:
    
    /** * THE OUTBOX: Called by GameInstance during a Save.
     * The Actor must pack its variables into the FActorRecord and return it.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveSystem")
    FActorRecord GetSaveData();

    /** * THE INBOX: Called by GameInstance during a Load.
     * The Actor receives its own data back and must apply it (Set Health, Open Door, etc).
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveSystem")
    void LoadSaveData(const FActorRecord& Record);
};
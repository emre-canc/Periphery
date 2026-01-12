#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ConfigurableInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UConfigurableInterface : public UInterface
{
    GENERATED_BODY()
};

class INSIDETFV03_API IConfigurableInterface
{
    GENERATED_BODY()

public:

    /** * Pushes a Data Asset to the actor to change its state or behavior.
     * @param Asset      The payload (Conversation, Video, Sound, KeyCode, etc). Can be null to "Clear".
     * @param ContextTag The instruction (e.g., "Phone.IncomingCall", "TV.ChannelChange", "Door.SetCode").
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Config")
    void ReceiveConfiguration(const UPrimaryDataAsset* Asset, FGameplayTag InstructionTag);
};
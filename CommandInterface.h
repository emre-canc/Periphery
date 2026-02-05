#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "CommandInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCommandInterface : public UInterface
{
    GENERATED_BODY()
};

class INSIDETFV03_API ICommandInterface
{
    GENERATED_BODY()

public:

    // Generic Command Handler (e.g. "Trigger", "Reset", "Open")
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Command")
    void ReceiveCommand(FGameplayTag CommandTag);
    
};
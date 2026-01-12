
#pragma once

#include "CoreMinimal.h"
#include "InteractionStructs.generated.h"

// 1. What the interactor is doing
UENUM(BlueprintType)
enum class EInteractorStates : uint8
{
    Free,
    Holding,
    Carrying,
    Inspecting,
    Busy
};

// 2. INPUT TYPE 
UENUM(BlueprintType)
enum class EInteractorInputs : uint8
{
    Press,
    Hold,
    PressAndHold    UMETA(DisplayName = "Press & Hold")

};

// 3. INTERACTION TYPE
UENUM(BlueprintType)
enum class EInteractions : uint8
{
    Interact,
    Inspect,
    Carry,
    Drop,
    Climb,
    None    UMETA(DisplayName = "None/Disabled")
};



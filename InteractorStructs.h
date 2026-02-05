
#pragma once

#include "CoreMinimal.h"
#include "InteractorStructs.generated.h"

// 1. What the interactor is doing
UENUM(BlueprintType)
enum class EInteractorStates : uint8
{
    Free,
    Equipped,
    Carrying,
    Inspecting,
    Busy
};







#pragma once

#include "CoreMinimal.h"
#include "InteractableStructs.generated.h"


// 1. INTERACTION TYPE
UENUM(BlueprintType)
enum class EInteractions : uint8
{
    Interact,
    Inspect UMETA(DisplayName="Inspect/Read"),
    Carry,
    Drop,
    Climb,
    None    UMETA(DisplayName = "None/Disabled")
};


// ATTACH DATA
UENUM(BlueprintType)
enum class EHandGripType : uint8
{
    None,
    Cylinder,
    Pinch,
    Flat   UMETA(DisplayName = "Flat (Carry)"),
    Weapon,
    Universal   UMETA(DisplayName = "Universal (Clipboard)")
};


USTRUCT(BlueprintType)
struct FHandAttachData
{
    GENERATED_BODY()

    // Which bone/socket on the Character mesh to attach to?
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AttachSocket = FName("Hand_L_Socket");

    // Local offset to align the mesh perfect (e.g., handle in palm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform RelativeTransform;

    // How should the character animate while holding this?
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHandGripType GripType = EHandGripType::None;
};


USTRUCT(BlueprintType)
struct FCachedItemState
{
    GENERATED_BODY()

    // Items collision profile before interaction.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CollisionProfileName = FName("BlockAll");

    // Whether item has physics.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSimulatePhysics = false;

    // The original location the item was before interaction.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform OriginalTransform;
};


USTRUCT(BlueprintType)
struct FInspectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform InspectTransform;
};


UENUM(BlueprintType)
enum class EInteractInput : uint8
{
    Press,
    Hold,
    DoublePress    UMETA(DisplayName = "Double Press"),
    PressAndHold    UMETA(DisplayName = "Press & Hold")

};
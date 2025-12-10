
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WidgetStructs.generated.h"


UENUM(BlueprintType)
enum class EWidgetType : uint8
{
    HUD        UMETA(DisplayName="HUD"),        // (The persistent layer)
    Menu       UMETA(DisplayName="Menu"),       //  (Pause screens, Inventory)
    Interactive UMETA(DisplayName="Interactive"), // Minigames, Keypads
    Passive    UMETA(DisplayName="Passive")     // Notifications, Subtitles
};


UENUM(BlueprintType)
enum class EWidgetMode : uint8
{
    Modal     UMETA(DisplayName="Modal"),
    Overlay   UMETA(DisplayName="Overlay"),
    Menu      UMETA(DisplayName="Menu")
};

UENUM(BlueprintType)
enum class EWidgetProgression : uint8
{
    Manual     UMETA(DisplayName="Manual"),
    Automatic  UMETA(DisplayName="Automatic")
};

UENUM(BlueprintType)
enum class EWidgetPriority : uint8
{
    Low     UMETA(DisplayName="Low"),
    Medium  UMETA(DisplayName="Medium"),
    High    UMETA(DisplayName="High"),
    Urgent  UMETA(DisplayName="Urgent")
};

USTRUCT(BlueprintType)
struct FWidgetData
{
    GENERATED_BODY()

    UPROPERTY(Transient, BlueprintReadOnly) 
    TWeakObjectPtr<UUserWidget> Widget;

    UPROPERTY(BlueprintReadWrite, EditAnywhere) 
    EWidgetType Type = EWidgetType::Passive;

    UPROPERTY(BlueprintReadWrite, EditAnywhere) 
    EWidgetMode Mode = EWidgetMode::Overlay;

    UPROPERTY(BlueprintReadWrite, EditAnywhere) 
    EWidgetProgression Progression = EWidgetProgression::Automatic;

    UPROPERTY(BlueprintReadWrite, EditAnywhere) 
    EWidgetPriority Priority = EWidgetPriority::Low;

    UPROPERTY(BlueprintReadWrite, EditAnywhere) 
    FName ContextTag = NAME_None;


    //Constructers used for C++, commented because it is not needed but good to have.
    // FWidgetData() {} 
    // FWidgetData(UUserWidget* InWidget, EWidgetType InType)
    //     : Widget(InWidget), Type(InType) {}
};


// /** Used for WBP_Text */ IF EVER NEEDED IN THE FUTURE
// USTRUCT(BlueprintType)
// struct FTextData
// {
// 	GENERATED_BODY()
// public:
// 	/** Please add a variable description */
// 	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Text", MakeStructureDefaultValue="Sample."))
// 	FString Text;

// 	/** Please add a variable description */
// 	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Text Color", MakeStructureDefaultValue="(SpecifiedColor=(R=1.000000,G=1.000000,B=1.000000,A=1.000000),ColorUseRule=UseColor_Specified)"))
// 	FSlateColor TextColor;

// 	/** Please add a variable description */
// 	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="TextDuration", MakeStructureDefaultValue="3.000000"))
// 	double TextDuration;

// 	/** Please add a variable description */
// 	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Typewriter?", MakeStructureDefaultValue="False"))
// 	bool bTypewriter;

// 	/** Please add a variable description */
// 	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="TypewriterSpeed", MakeStructureDefaultValue="0.050000"))
// 	double TypewriterSpeed;
// };


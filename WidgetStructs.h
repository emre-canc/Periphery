
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WidgetStructs.generated.h"

// 1. VISUAL LAYER (Where does it draw?)
UENUM(BlueprintType)
enum class EWidgetLayer : uint8
{
    Game        UMETA(DisplayName="Game (HUD)"),       // Z-Order 10
    Menu        UMETA(DisplayName="Menu (Settings)"),  // Z-Order 50
    Modal       UMETA(DisplayName="Modal (Popup)"),    // Z-Order 100
    System      UMETA(DisplayName="System (Toast)")    // Z-Order 200
};

// 2. INPUT MODE (Who gets the buttons?)
UENUM(BlueprintType)
enum class EWidgetInputMode : uint8
{
    // Input goes to Player Controller. UI is ignored.
    GameOnly        UMETA(DisplayName="Game Only"),

    // Input goes to UI, Player Controller is ignored.
    UIOnly          UMETA(DisplayName="UI Only"),
    

    // If the UI ignores input, it falls through to the Player Controller.
    GameAndUI       UMETA(DisplayName="Game and UI")
    
};

USTRUCT(BlueprintType)
struct FWidgetData
{
    GENERATED_BODY()

    UPROPERTY(Transient, BlueprintReadOnly) 
    TWeakObjectPtr<UUserWidget> Widget;

    // --- VISUALS ---
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Config") 
    EWidgetLayer Layer = EWidgetLayer::Game;

    // --- BEHAVIOR ---
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Config") 
    EWidgetInputMode InputMode = EWidgetInputMode::UIOnly;

    // Do we see the mouse? (SEPARATE from Input Mode)
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Config") 
    bool bShowMouseCursor = false;

    // Do we freeze the world?
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Config") 
    bool bPauseGame = false;

    // Tag for finding this widget later
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Config") 
    FName ContextTag = NAME_None;

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


#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputMappingContext.h"
#include "PeripheryWidgetSettings.generated.h"

/**
 * Global Settings for the UI System.
 * Editable via Project Settings -> Game -> Periphery UI Settings
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Periphery Widget Settings"))
class INSIDETFV03_API UPeripheryUISettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    // The Input Mapping Context to apply when a Menu is open
    UPROPERTY(Config, EditAnywhere, Category="Input", meta=(DisplayName="UI Input Context"))
    TSoftObjectPtr<UInputMappingContext> DefaultUIInputContext;
};
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "ElectricityInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UElectricityInterface : public UInterface
{
    GENERATED_BODY()
};

class INSIDETFV03_API IElectricityInterface
{
    GENERATED_BODY()

public:
    // --- Activate Category (Player/Interaction) ---
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Electricity|Activate")
    void TurnOn();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Electricity|Activate")
    void TurnOff();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Electricity|Activate")
    void ToggleOnOff();

    // Note: For "Output" pins in BP, use a reference (&) in C++
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Electricity|Activate")
    void IsActive(bool& bIsActivated) const;

    // --- Flicker Category ---

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Electricity|Flicker")
    void StartFlicker();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Electricity|Flicker")
    void StopFlicker();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Electricity|Flicker")
    void IsFlickering(bool& bIsFlickering) const;

    // --- Power Category (Grid/Subsystem) ---

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Electricity|Power")
    void PowerOn();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Electricity|Power")
    void PowerOff();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Electricity|Power")
    void IsPowered(bool& bIsPowered) const;

    // --- System ---
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Electricity|Circuit")
    void GetCircuitTag(FGameplayTag& CircuitTag);
};
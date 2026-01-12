#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Interfaces/ElectricityInterface.h"
#include "ElectricitySubsystem.generated.h"


UCLASS()
class INSIDETFV03_API UElectricitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    // ---------- CONTROL ----------

    /** * Turns a specific circuit ON or OFF.
     * Updates internal state and notifies all actors on that circuit.
     * Checks hierarchy: You cannot turn a Child Grid ON if the Parent Grid is OFF.
     */
    UFUNCTION(BlueprintCallable, Category="Electricity|Circuit")
    void SetCircuitState(FGameplayTag CircuitTag, bool bPowerOn);

    // ---------- QUERIES ----------

    UFUNCTION(BlueprintPure, Category="Electricity|Circuit")
    bool IsCircuitOn(FGameplayTag CircuitTag) const;
    
    // Returns EVERYTHING connected to the grid (Sources, Lights, Switches)
    UFUNCTION(BlueprintPure, Category="Electricity")
    TArray<AActor*> GetGridActors() const;

    // Returns only the Lights (Convenience function)
    UFUNCTION(BlueprintPure, Category="Electricity")
    TArray<AActor*> GetLightActors() const;

    // Returns Lights specifically in a target room (e.g., "Location.Store.Basement")
    UFUNCTION(BlueprintPure, Category="Electricity")
    TArray<AActor*> GetLightsInRoom(FGameplayTag RoomTag) const;

    // ---------- SAVE / LOAD ----------

    // Call this when saving the game
    const TMap<FGameplayTag, bool>& GetCircuitStates() const { return CircuitStates; }

    // Call this when loading the game
    void RestoreCircuitStates(const TMap<FGameplayTag, bool>& LoadedStates);

    // ---------- DEBUG CONSOLE COMMANDS ----------

    UFUNCTION(Exec) 
    void SetCircuitDebug(FName CircuitTagName, bool bOn);

    UFUNCTION(Exec)
    void KillAllPower();

    UFUNCTION(Exec)
    void RestoreAllPower();

private:

    // Tracks the current state of every grid. If a tag is missing, we assume it is ON.
    UPROPERTY()
    TMap<FGameplayTag, bool> CircuitStates;
    
};
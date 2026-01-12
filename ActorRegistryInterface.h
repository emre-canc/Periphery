#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "ActorRegistryInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UActorRegistryInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for actors to self-report their Identity and Location to the Registry.
 */
class INSIDETFV03_API IActorRegistryInterface
{
    GENERATED_BODY()

public:

    // Identity: What am I? (e.g. "Electricity.Consumer", "Camera.Security")
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Registry")
    FGameplayTag GetActorTag() const;

    // Location: Where am I? (e.g. "Location.Kitchen", "Location.Basement")
    // Critical for the Electricity Intersection logic.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Registry")
    FGameplayTag GetLocationTag() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Registry")
    FGameplayTagContainer GetAllTags() const;

};

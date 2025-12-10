
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InspectionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class INSIDETFV03_API UInspectionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()


public:

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    AActor* GetInspector() const;

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    AActor* GetInspectedActor() const;

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    bool IsInspecting() const { return bIsInspecting; }
    

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    void SetInspector(AActor* NewInspectorActor);

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    void SetInspectedActor(AActor* NewInspectedActor);

    UFUNCTION(BlueprintCallable, Category="LevelState|Inspector")
    void SetIsInspecting(bool bNewIsInspecting); 

private:

    TWeakObjectPtr<AActor> InspectorActor = nullptr;
    TWeakObjectPtr<AActor> InspectedActor = nullptr;
    bool bIsInspecting = false;
};

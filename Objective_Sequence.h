#pragma once
#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "Objective_Sequence.generated.h"

USTRUCT(BlueprintType)
struct FStepRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Step")
	FGameplayTag RequiredEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Step")
	bool RequireUniqueSources;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Step")
	int32 NumberOfTimesEventMustOccur = 1;
};

USTRUCT(BlueprintType)
struct FObjectiveStepDefinition
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, Category = "Config")
    FName StepID;

    UPROPERTY(EditAnywhere, Category = "Config")
    FText StepDescription;

    // Runs when this specific step becomes active
    UPROPERTY(EditAnywhere, Instanced, Category = "Actions")
    TArray<TObjectPtr<UMissionAction>> StepStartActions;
	
    UPROPERTY(EditAnywhere, Category = "Config")
    TArray<FStepEventRequirement> RequiredEventsToCompleteStep;

    // Runs when this specific step is completed
    UPROPERTY(EditAnywhere, Instanced, Category = "Actions")
    TArray<TObjectPtr<UMissionAction>> StepCompleteActions;
};

UCLASS(DisplayName = "Sequence (Ordered Steps)")
class INSIDETFV03_API UObjective_Sequence : public UMissionObjective
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Rules")
    TArray<FObjectiveStepDefinition> Steps;

private:

void RunStepActions(const TArray<TObjectPtr<UMissionAction>>& Actions, AActor* Context) const;

};


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

	UPROPERTY(EditAnywhere, Instanced, Category = "Actions")
	TArray<TObjectPtr<UMissionAction>> StepStartActions;

	UPROPERTY(EditAnywhere, Instanced, Category = "Actions")
	TArray<TObjectPtr<UMissionAction>> StepCompleteActions;
	
    UPROPERTY(EditAnywhere, Category = "Config")
    TArray<FStepRequirement> RequiredEventsToCompleteStep;

};

UCLASS(DisplayName = "Sequence (Ordered Steps)")
class INSIDETFV03_API UObjective_Sequence : public UMissionObjective
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Rules")
    TArray<FObjectiveStepDefinition> Steps;
	
	virtual bool OnEvent(const FGameplayTag& MissionID, const FGameplayTag& EventTag, AActor* SourceActor, FObjectiveRuntimeState& RuntimeState) const override;
    
    virtual bool IsComplete(const FObjectiveRuntimeState& RuntimeState) const override;

private:

void RunStepActions(const TArray<TObjectPtr<UMissionAction>>& Actions, AActor* Context) const;

};


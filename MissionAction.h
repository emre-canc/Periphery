#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Subsystems/ActorRegistrySubsystem.h"
#include "MissionAction.generated.h"


// Abstract base for all actions (Start, Complete, Step Actions)
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class INSIDETFV03_API UMissionAction : public UObject
{
    GENERATED_BODY()

public:
    // The main entry point. 
    // We pass ContextActor so the Action can find the World/GameInstance.
    virtual void ExecuteAction(AActor* ContextActor) const {};
};



//**----------------------*//
UCLASS(DisplayName = "Send Command To Actor")
class INSIDETFV03_API UAction_SendCommand : public UMissionAction
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Config")
    FGameplayTag TargetActorTag;

    UPROPERTY(EditAnywhere, Category = "Config")
    FName CommandName;

    virtual void ExecuteAction(AActor* ContextActor) const override;
	
};

UCLASS(DisplayName = "Play/Stop Level Sequence")
class INSIDETFV03_API UAction_Sequence : public UMissionAction
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Config")
    FGameplayTag SequenceTag;

    virtual void ExecuteAction(AActor* ContextActor) const override;

};



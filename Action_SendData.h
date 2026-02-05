#pragma once

#include "CoreMinimal.h"
#include "Subsystems/ActorRegistrySubsystem.h"
#include "Missions/Actions/MissionAction.h"
#include "Action_SendData.generated.h"


UCLASS(DisplayName = "Send Data To Actor")
class INSIDETFV03_API UAction_SendData : public UMissionAction
{
    GENERATED_BODY()

public:
// 1. Who are we talking to? (e.g. "Prop.Installation.Phone")
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    FGameplayTag ActorTag;

    // 2. What are we sending? (e.g. DA_BossConversation, DA_StaticVideo, nullptr)
    // We use UPrimaryDataAsset to allow any child class (Conversation, Media, etc).
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    TObjectPtr<UPrimaryDataAsset> Payload;

    // 3. How should they interpret it? (e.g. "Phone.IncomingCall" vs "Phone.SetVoicemail")
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    FGameplayTag ContextTag;

    virtual void ExecuteAction(AActor* ContextActor) const override;
};


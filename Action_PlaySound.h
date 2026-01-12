#pragma once

#include "CoreMinimal.h"
#include "Missions/Actions/MissionAction.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "Action_PlaySound.generated.h"

UENUM(BlueprintType)
enum class EAudioPlaybackMode : uint8
{
    OneShot     UMETA(DisplayName = "One Shot"),
    Cue         UMETA(DisplayName = "Cue (Randomizer)")
};

UCLASS(DisplayName = "Play Sound")
class INSIDETFV03_API UAction_PlaySound : public UMissionAction
{
    GENERATED_BODY()

public:
    virtual void ExecuteAction(AActor* ContextActor) const override;

protected:

    UPROPERTY(EditAnywhere, Category = "Audio Settings")
    EAudioPlaybackMode PlaybackMode = EAudioPlaybackMode::OneShot;

    UPROPERTY(EditAnywhere, Category = "Audio Settings")
    USoundBase* MetaSoundBase;

    // --- ONE SHOT DATA ---

    UPROPERTY(EditAnywhere, Category = "Audio Data", meta = (EditCondition = "PlaybackMode == EAudioPlaybackMode::OneShot", EditConditionHides))
    USoundWave* SoundWave;

    UPROPERTY(EditAnywhere, Category = "Audio Data", meta = (EditCondition = "PlaybackMode == EAudioPlaybackMode::OneShot", EditConditionHides))
    float Pitch = 1.0f;

    // --- CUE (RANDOMIZER) DATA ---

    UPROPERTY(EditAnywhere, Category = "Audio Data", meta = (EditCondition = "PlaybackMode == EAudioPlaybackMode::Cue", EditConditionHides))
    TArray<USoundWave*> SoundPool;

    UPROPERTY(EditAnywhere, Category = "Audio Data", meta = (EditCondition = "PlaybackMode == EAudioPlaybackMode::Cue", EditConditionHides))
    float MinPitch = 0.8f;

    UPROPERTY(EditAnywhere, Category = "Audio Data", meta = (EditCondition = "PlaybackMode == EAudioPlaybackMode::Cue", EditConditionHides))
    float MaxPitch = 1.2f;
};
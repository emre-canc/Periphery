#include "Missions/Actions/Action_PlaySound.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

void UAction_PlaySound::ExecuteAction(AActor* ContextActor) const
{
    if (!ContextActor || !MetaSoundBase) return;

    // 1. Spawn the Sound (Fire and Forget)
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        ContextActor,
        MetaSoundBase,
        ContextActor->GetActorLocation()
    );

    if (!AudioComp) return;

    // 2. Inject Data
    switch (PlaybackMode)
    {
    case EAudioPlaybackMode::OneShot:
        if (SoundWave)
        {
            // Send the single wave
            AudioComp->SetObjectParameter(FName("WaveInput"), SoundWave);

            // Send the specific pitch
            AudioComp->SetFloatParameter(FName("Pitch"), Pitch);
        }
        break;

    case EAudioPlaybackMode::Cue:
        if (SoundPool.Num() > 0)
        {
            // A. Handle the Array (Convert USoundWave* to UObject*)
            TArray<UObject*> ObjectPool;
            for (USoundWave* Wave : SoundPool)
            {
                ObjectPool.Add(Wave);
            }
            AudioComp->SetObjectArrayParameter(FName("WaveArrayInput"), ObjectPool);


            AudioComp->SetFloatParameter(FName("MinPitch"), MinPitch);
            AudioComp->SetFloatParameter(FName("MaxPitch"), MaxPitch);
    }
        break;
    }
}
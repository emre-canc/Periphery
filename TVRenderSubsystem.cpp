#include "Subsystems/TVRenderSubsystem.h"
#include "Kismet/KismetRenderingLibrary.h"

UTextureRenderTarget2D* UTVRenderSubsystem::RegisterTV(AActor* TVActor, UMaterialInterface* SourceMaterial)
{
    if (!SourceMaterial || !TVActor) return nullptr;

    // 1. Check if this feed already exists
    FTVFeedData* FeedData = ActiveFeeds.Find(SourceMaterial);

    if (FeedData)
    {
        // Feed exists: Just add this TV to the watchers list
        FeedData->Watchers.Add(TVActor);
        return FeedData->RenderTarget;
    }

    // 2. Feed does NOT exist: Create new resources
    UTextureRenderTarget2D* NewRT = NewObject<UTextureRenderTarget2D>(this);
    NewRT->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
    NewRT->InitAutoFormat(640, 480);
    NewRT->UpdateResource();

    // 3. Create the data entry
    FTVFeedData NewData;
    NewData.RenderTarget = NewRT;
    NewData.Watchers.Add(TVActor);

    // 4. Add to Map
    ActiveFeeds.Add(SourceMaterial, NewData);

    return NewRT;
}

void UTVRenderSubsystem::UnregisterTV(AActor* TVActor, UMaterialInterface* SourceMaterial)
{
    if (!SourceMaterial || !TVActor) return;

    if (FTVFeedData* FeedData = ActiveFeeds.Find(SourceMaterial))
    {
        // 1. Remove this specific TV from the watchers
        FeedData->Watchers.Remove(TVActor);

        // 2. If NOBODY is watching anymore, clean up
        if (FeedData->Watchers.IsEmpty())
        {
            // Optional: Explicitly release resource if you want to be aggressive with memory
            // FeedData->RenderTarget->ReleaseResource(); 
     
            // Remove the entry. The Subsystem stops Ticking this material.
            ActiveFeeds.Remove(SourceMaterial);
        }
    }
}

void UTVRenderSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;
  
    // Iterate over all active feeds and paint them
    for (auto& Pair : ActiveFeeds)
    {
        UMaterialInterface* Mat = Pair.Key;
        UTextureRenderTarget2D* RT = Pair.Value.RenderTarget;
    
        // Safety check
        if (Mat && RT)
        {
            UKismetRenderingLibrary::DrawMaterialToRenderTarget(World, RT, Mat);
        }
    }
}

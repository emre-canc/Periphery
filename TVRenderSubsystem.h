#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInterface.h"
#include "TVRenderSubsystem.generated.h"

// A simple struct to hold the Texture + Who is watching it
USTRUCT()
struct FTVFeedData
{
    GENERATED_BODY()
    
    // The Output Texture
    UPROPERTY()
    TObjectPtr<UTextureRenderTarget2D> RenderTarget;

    // The list of TVs watching this feed (Using a Set prevents duplicates)
    TSet<AActor*> Watchers;
};

UCLASS()
class INSIDETFV03_API UTVRenderSubsystem : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()
    
public:
    // Called when a TV turns ON or switches channel. Returns the Texture to display.
    UFUNCTION(BlueprintCallable, Category = "TV System")
    UTextureRenderTarget2D* RegisterTV(AActor* TVActor, UMaterialInterface* SourceMaterial);

    // Called when a TV turns OFF or is destroyed.
    UFUNCTION(BlueprintCallable, Category = "TV System")
    void UnregisterTV(AActor* TVActor, UMaterialInterface* SourceMaterial);

    // -- FTickableGameObject Interface --
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override { return TStatId(); }
    virtual bool IsTickable() const override { return !ActiveFeeds.IsEmpty(); }
    // -----------------------------------

private:
    // Maps the Material -> The Data (Texture + Watcher List)
    TMap<UMaterialInterface*, FTVFeedData> ActiveFeeds;
}; 

// Fill out your copyright notice in the Description page of Project Settings.


#include "TVRenderSubsystem.h"
#include "TVRenderTargetBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"


UTVRenderSubsystem::UTVRenderSubsystem()
{
    static ConstructorHelpers::FClassFinder<ATVRenderTargetBase> RendererBP(TEXT("/Game/Items/TVs/BP_TVRenderTarget.BP_TVRenderTarget_C")); // Adjust this path
    if (RendererBP.Succeeded())
    {
        TVRenderTargetClass = RendererBP.Class;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find BP_TargetRenderTarget class, wrong path!"));
    }
}

ATVRenderTargetBase* UTVRenderSubsystem::InitializeTVScreenRender(UMaterialInstance* RenderMaterial)
{
    if (!RenderMaterial) return nullptr; 
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    if (ActiveTVRenderTarget.Contains(RenderMaterial))
{
        return ActiveTVRenderTarget[RenderMaterial];
} else {
    const float StartX = 10000.f;
    const float StepX  = 1000.f;
    FVector Location(StartX + StepX * ActiveTVRenderTarget.Num(), 0.f, 0.f);

    if (!TVRenderTargetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("TVRenderTargetClass is null!"));
        return nullptr;
    } else {
        ATVRenderTargetBase* NewRenderer = World->SpawnActor<ATVRenderTargetBase>(
        TVRenderTargetClass, Location, FRotator::ZeroRotator);

        NewRenderer->RenderMesh->SetMaterial(0, RenderMaterial);
        NewRenderer->RenderMaterial = RenderMaterial;
        ActiveTVRenderTarget.Add(RenderMaterial, NewRenderer);
     
        return NewRenderer;
        }
}

  
    
}

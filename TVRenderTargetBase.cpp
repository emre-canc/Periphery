// Fill out your copyright notice in the Description page of Project Settings.


#include "TVRenderTargetBase.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"


// Sets default values
ATVRenderTargetBase::ATVRenderTargetBase()
{
  PrimaryActorTick.bCanEverTick = false;

    RenderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RenderMesh"));
    RootComponent = RenderMesh;
	RenderCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("RenderCamera"));
	RenderCamera->SetupAttachment(RootComponent);
	
}

UTextureRenderTarget2D* ATVRenderTargetBase::GetRenderTexture()
{
	if (!RenderCamera->TextureTarget)
	{
		RenderCamera->TextureTarget = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass(), TEXT("RenderTarget"));
		RenderCamera->TextureTarget->InitAutoFormat(640, 480); // Set the desired resolution 
		//To avoid distortion: Mesh width ≈ 2DSceneCapture Ortho Width & Mesh aspect ratio ≈ Render target aspect ratio
		RenderCamera->TextureTarget->UpdateResource();
		return RenderCamera->TextureTarget;
	} else {
		return RenderCamera->TextureTarget;
	}
}

// Called when the game starts or when spawned
void ATVRenderTargetBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATVRenderTargetBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



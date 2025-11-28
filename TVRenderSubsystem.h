// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TVRenderTargetBase.h"
#include "TVRenderSubsystem.generated.h"

/**
 *
 */
UCLASS()
class INSIDETFV03_API UTVRenderSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
    
public:

    UTVRenderSubsystem();
    UPROPERTY(EditDefaultsOnly, Category = "Default")
    TSubclassOf<ATVRenderTargetBase> TVRenderTargetClass;

    /** Gets or creates TVRenderTargetBase */
    UFUNCTION(BlueprintCallable)
    ATVRenderTargetBase* InitializeTVScreenRender(UMaterialInstance* RenderMaterial);
    
    /** Initializes Material Instance&BP_RenderTarget Map variable */
    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
    TMap<UMaterialInstance*, ATVRenderTargetBase*> ActiveTVRenderTarget;


    
}; 

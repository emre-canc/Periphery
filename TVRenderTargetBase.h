// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TVRenderTargetBase.generated.h"

UCLASS()
class INSIDETFV03_API ATVRenderTargetBase : public AActor
{
	GENERATED_BODY()
public:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
UTextureRenderTarget2D* GetRenderTexture();

public:	
	// Sets default values for this actor's properties
	ATVRenderTargetBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* RenderMesh;

	/** Please add a variable description */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneCaptureComponent2D* RenderCamera;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default", meta=(ExposeOnSpawn="true"))
	TObjectPtr<UMaterialInstance> RenderMaterial;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

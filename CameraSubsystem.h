#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "CameraSubsystem.generated.h"

/**
 *
 */
UCLASS()
class INSIDETFV03_API UCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Camera|Blend")
	void BlendToCamera(FGameplayTag Tag, float BlendTime);

	UFUNCTION(BlueprintCallable, Category = "Camera|Blend")
	void BlendToAndBack(FGameplayTag Tag, float BlendTime, float WaitTime);

	UFUNCTION(BlueprintCallable, Category = "Camera|Blend")
	void BlendToPlayer(float BlendTime);

	UFUNCTION(BlueprintCallable, Category = "Camera|Blend")
	void FadeToCamera(FGameplayTag Tag, float FadeTime, float WaitTime);

	UFUNCTION(BlueprintCallable, Category = "Camera|Blend")
	bool CutToCamera(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Camera|Data")
	TArray<AActor*> GetCamerasByTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Camera|Data")
	TArray<AActor*> GetCameras(FGameplayTag Tag) const;



private:

    FTimerHandle TimerHandle_CameraWork;

};
 
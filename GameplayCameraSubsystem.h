#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "GameplayCameraSubsystem.generated.h"

/**
 *
 */
UCLASS()
class INSIDETFV03_API UGameplayCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CameraSubsystem")
	void RegisterCamera(FGameplayTag Tag, AActor *Actor);

	UFUNCTION(BlueprintCallable, Category = "CameraSubsystem")
	void UnregisterCamera(AActor *Actor);

	UFUNCTION(BlueprintCallable, Category = "CameraSubsystem")
	void BlendTo(FGameplayTag Tag, float BlendTime);

	UFUNCTION(BlueprintCallable, Category = "CameraSubsystem")
	void BlendToAndBack(FGameplayTag Tag, float BlendTime, float WaitTime);

	UFUNCTION(BlueprintCallable, Category = "CameraSubsystem")
	void BlendToPlayer(float BlendTime);

	UFUNCTION(BlueprintCallable, Category = "CameraSubsystem")
	TArray<AActor *> FindCameras(FGameplayTag Tag) const;



private:

	TMap<FGameplayTag, TArray<TWeakObjectPtr<AActor>>> CamerasByTag;
	// reverse index for fast unregister

	TMap<TWeakObjectPtr<AActor>, FGameplayTag> TagToActor;


};
 
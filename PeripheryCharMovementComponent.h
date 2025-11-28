// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PeripheryCharMovementComponent.generated.h"

UENUM()
enum class ECustomMovementMode : uint8
{
    CMOVE_None   UMETA(Hidden),
    CMOVE_Ladder = 1
};

class ABaseLadder;

UCLASS()
class INSIDETFV03_API UPeripheryCharMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	public: 
	UFUNCTION(BlueprintCallable, Category="Movement|Ladder")
	void StartLadder(ABaseLadder* InLadder);

	UFUNCTION(BlueprintCallable, Category="Movement|Ladder")
	void StopLadder(bool bAtTop);

	UFUNCTION(BlueprintCallable, Category="Movement|Ladder")
	void LadderInput(float Value);

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    float ClimbSpeed = 200.f;         // cm/s

protected:
    virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
    void PhysLadder(float DeltaTime, int32 Iterations);


private:

    // Runtime ladder state
    TWeakObjectPtr<ABaseLadder> CurrentLadder;
	FVector LadderTop;
	FVector LadderBottom;
	FVector LadderDir;
	float LadderLength = 0.f;
	float LadderT = 0.f;

    float ClimbInput = 0.f;           // between -1..+1

    // Helpers
    void EnterLadderMode();
    void ExitLadderMode();
};

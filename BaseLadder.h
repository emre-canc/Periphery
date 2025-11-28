// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseLadder.generated.h"

UCLASS()
class INSIDETFV03_API ABaseLadder : public AActor
{
    GENERATED_BODY()


    
public:
    // Sets default values for this actor's properties
    ABaseLadder();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* LadderRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    USceneComponent* LadderTop;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    USceneComponent* LadderBottom;

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Ladder")   
    void UpdateClimbStatus(bool bClimbStatus);

    FVector GetLadderTop() const;
    FVector GetLadderBottom() const;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

};

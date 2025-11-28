// Fill out your copyright notice in the Description page of Project Settings.


#include "PeripheryCharMovementComponent.h"
#include "GameFramework/Actor.h"
#include "BaseLadder.h"


// Flow:    Player interacts with ladder. The ladder turns the player movement to ladder(gravity off, only moving up and down), 
//              teleports the player closer if its not close enough. The ladder sends its top and bottom to player movement, 
//              and it calculates ladder completion (0-1). The player sends input axis and its clamped between -1 to 1, when on top the player gets teleported forward, 
//              movement mode is restored. 
//


void UPeripheryCharMovementComponent::LadderInput(float Value)
{
    ClimbInput = FMath::Clamp(Value, -1.f, 1.f);
}

void UPeripheryCharMovementComponent::StartLadder(ABaseLadder* InLadder)
{
    if (!InLadder || !UpdatedComponent) return;
    CurrentLadder = InLadder;

    LadderTop = CurrentLadder->GetLadderTop();
    LadderBottom = CurrentLadder->GetLadderBottom();
    LadderDir = LadderTop - LadderBottom;
    LadderLength = LadderDir.Size();
    if (LadderLength < KINDA_SMALL_NUMBER) return;

    FVector PlayerPosition = UpdatedComponent->GetComponentLocation();
    FVector PlayerAlignment = PlayerPosition - LadderBottom;
    LadderT = FMath::Clamp(FVector::DotProduct(PlayerAlignment, LadderDir) / LadderDir.SizeSquared(), 0.f, 1.f);
    const FVector SnapTargetToPos = LadderBottom + LadderDir * LadderT;
    UpdatedComponent->SetWorldLocation(SnapTargetToPos, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);

    if (PawnOwner && PawnOwner->GetController() && LadderT <= 0.f + KINDA_SMALL_NUMBER) // sets player rotation to ladder
    {
        FVector LadderForward = FVector::CrossProduct(LadderDir, FVector::ForwardVector).GetSafeNormal();
        PawnOwner->SetActorRotation(LadderForward.Rotation());
        PawnOwner->GetController()->SetControlRotation(LadderForward.Rotation());
    } 


    EnterLadderMode();
}

void UPeripheryCharMovementComponent::StopLadder(bool /*bAtTop*/)
{
    ExitLadderMode();
    CurrentLadder.Reset();
}

void UPeripheryCharMovementComponent::PhysCustom(float DeltaTime, int32 Iterations) 
{
    switch(CustomMovementMode)
    {
        case (uint8)ECustomMovementMode::CMOVE_Ladder:
            PhysLadder(DeltaTime, Iterations);
            break;

        default: 
            Super::PhysCustom(DeltaTime, Iterations);
            break;
    }
}

void UPeripheryCharMovementComponent::PhysLadder(float DeltaTime, int32 Iterations)
{
    if (!CurrentLadder.IsValid() || !UpdatedComponent || LadderLength < KINDA_SMALL_NUMBER) return;

    LadderT = FMath::Clamp((ClimbInput * (ClimbSpeed * DeltaTime / LadderLength) + LadderT), 0.f, 1.f);

    if ((LadderT >=  1.f - KINDA_SMALL_NUMBER && ClimbInput > 0.f) || (LadderT <= 0.f + KINDA_SMALL_NUMBER && ClimbInput < 0.f) )
    {
        bool bAtTop = LadderT >= 1.f - KINDA_SMALL_NUMBER;
        StopLadder(bAtTop);
        return;
    }

    FVector TargetPos = LadderBottom + LadderDir * LadderT;

    // Move capsule
    FHitResult Hit;
    FVector CurrentPos = UpdatedComponent->GetComponentLocation();
    SafeMoveUpdatedComponent(TargetPos - CurrentPos, UpdatedComponent->GetComponentQuat(), true, Hit);
}


void UPeripheryCharMovementComponent::EnterLadderMode()
{
    SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Ladder);
    GravityScale = 0;
    Velocity = FVector::ZeroVector;
    CurrentLadder->UpdateClimbStatus(true);
}


void UPeripheryCharMovementComponent::ExitLadderMode()
{
    SetMovementMode(MOVE_Walking);
    GravityScale = 1.f;
    ClimbInput = 0.f;
    if (CurrentLadder.IsValid()) { CurrentLadder->UpdateClimbStatus(false); }

}
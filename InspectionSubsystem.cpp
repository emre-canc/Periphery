
#include "Subsystems/InspectionSubsystem.h"
#include "GameFramework/Actor.h"



    // **INSPECTING**


void UInspectionSubsystem::SetInspector(AActor* NewInspectorActor)
{
        if (!IsValid(NewInspectorActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::SetInspector -> Invalid Inspector Actor"));
        return;
    }
    InspectorActor = NewInspectorActor;
}

void UInspectionSubsystem::SetInspectedActor(AActor* NewInspectedActor)
{
        if (!IsValid(NewInspectedActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::SetInspectedActor -> Inspected Actor is not valid"));
        return;
    }
    InspectedActor = NewInspectedActor;
}



AActor* UInspectionSubsystem::GetInspector() const
{
    if (!InspectorActor.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::GetInspector -> Inspector is not valid"));
        return nullptr;
    }
    return InspectorActor.Get();   
}


AActor* UInspectionSubsystem::GetInspectedActor() const
{
    if (!InspectedActor.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelStateSubsystem::GetInspectedActor -> Inspected Actor is not valid"));
        return nullptr;
    }
    return InspectedActor.Get();   
}

  
void UInspectionSubsystem::SetIsInspecting(bool bNewIsInspecting)
{   
    bIsInspecting = bNewIsInspecting;
    if (!bIsInspecting)
    {
        InspectedActor= nullptr;
    } // resets inspected actor after each inspect
}
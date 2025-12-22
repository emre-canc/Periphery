#include "Missions/Actions/Action_SaveGame.h"
#include "Core/PeripheryGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UAction_SaveGame::ExecuteAction(AActor* ContextActor) const
{
    // 1. Validate Context
    if (!ContextActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Action_SaveGame: Failed. ContextActor is null."));
        return;
    }

    // 2. Get Game Instance
    UGameInstance* GI = ContextActor->GetGameInstance();
    UPeripheryGameInstance* PeripheryGI = Cast<UPeripheryGameInstance>(GI);

    if (!PeripheryGI)
    {
        UE_LOG(LogTemp, Error, TEXT("Action_SaveGame: Failed. GameInstance is not UPeripheryGameInstance."));
        return;
    }

    // 3. Trigger Save
    // We ignore the return bool here, but you could log it if you wanted
    PeripheryGI->SaveGame(SlotName);
    
    UE_LOG(LogTemp, Log, TEXT("Action_SaveGame: Triggered Save for slot '%s'"), *SlotName);
}
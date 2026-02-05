// MissionSubsystem.cpp

#include "Subsystems/MissionSubsystem.h"
#include "Core/PeripherySaveGame.h"

#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceNull.h"


// ---------- Initialize ----------

void UMissionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Initialized"));

	FString NetMode = (GetWorld()->GetNetMode() == NM_Client) ? "Client" : "Server";
	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Init [%s]"), *NetMode);

}


// ---------- Mission control ----------

void UMissionSubsystem::StartMission(FGameplayTag MissionID)
{
	if (!MissionID.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: StartMission: Invalid MissionID"));
		return;
	}
	if (ActiveMissions.Contains(MissionID))
	{
		UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: StartMission: Mission already active: %s"), *MissionID.ToString());
		return;
	}

	//Constructing the Asset ID
	FPrimaryAssetId AssetId(UMissionData::StaticClass()->GetFName(), MissionID.GetTagName());
	UAssetManager& Manager = UAssetManager::Get();

	//Check if already loaded
	if (UMissionData* Mission = Cast<UMissionData>(Manager.GetPrimaryAssetObject(AssetId)))
	{
		OnMissionAssetLoaded(AssetId, MissionID); // start if loaded
	} 
	else 
	{
		//Request Async Load.
        UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: StartMission: Async loading asset for %s..."), *MissionID.ToString());

        // Create the delegate
        FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject
			(this, &UMissionSubsystem::OnMissionAssetLoaded,  AssetId, MissionID);

        // Ask Asset Manager to load it in the background
        Manager.LoadPrimaryAssets({ AssetId }, TArray<FName>(), Delegate);
	}
}

void UMissionSubsystem::FinishMission(FGameplayTag MissionID, bool bSuccess)
{
	FMissionRuntimeState* MissionRt = GetActiveMissionRuntime(MissionID);
	if (!MissionRt) return;

	MissionRt->MissionState = bSuccess ? EProgressState::Completed : EProgressState::Failed;

	// Move to completed archive
	CompletedMissions.Add(MissionID, *MissionRt);
	ActiveMissions.Remove(MissionID);
	OnMissionCompleted.Broadcast(MissionID, bSuccess);

	const UMissionData* MissionAsset = GetMissionAsset(MissionID);

	//Handle Next Mission
    if (bSuccess && MissionAsset && MissionAsset->NextMissionID.IsValid())
    {
        // StartMission will handle the Async loading for us
        StartMission(MissionAsset->NextMissionID);
    }

}

bool UMissionSubsystem::IsMissionActive(FGameplayTag MissionID) const
{
	const FMissionRuntimeState* Rt = GetActiveMissionRuntime(MissionID);
	return Rt && Rt->MissionState == EProgressState::InProgress;
}

void UMissionSubsystem::ActivateObjective(FGameplayTag MissionID, FGameplayTag ObjectiveID)
{
    // LOG: Entry
    UE_LOG(LogTemp, Log, TEXT("[Mission] Request Activate: %s (Mission: %s)"), *ObjectiveID.ToString(), *MissionID.ToString());

    // Check if valid section
    if (!IsMissionActive(MissionID)) 
    {
        UE_LOG(LogTemp, Warning, TEXT("[Mission] Failed to activate %s: Mission %s is not active."), *ObjectiveID.ToString(), *MissionID.ToString());
        return;
    }

    const UMissionObjective* ObjDef = GetObjectiveFromAsset(MissionID, ObjectiveID);
    if (!ObjDef) 
    {
        UE_LOG(LogTemp, Error, TEXT("[Mission] Failed to activate %s: Objective Definition not found in Asset."), *ObjectiveID.ToString());
        return;
    }

    // Is valid. Start Activate Objective section
    FMissionRuntimeState* MissionRt = GetActiveMissionRuntime(MissionID);
    check(MissionRt); // This ensures we crash hard if logic is broken (since we checked IsMissionActive above)

    // LOG: State Change
    UE_LOG(LogTemp, Display, TEXT("[Mission] ACTIVATED: %s added to ActiveObjectives."), *ObjectiveID.ToString());

    FObjectiveRuntimeState& ObjRt = MissionRt->ActiveObjectives.FindOrAdd(ObjectiveID);
    ObjRt.ObjectiveID = ObjectiveID;
    
    // 1. Delegate Initialization to the Object
    ObjDef->InitializeRuntime(ObjRt);

    // 2. Run Start Actions
    AActor* Context = GetGameInstance()->GetFirstLocalPlayerController()->GetPawn();
    
    if (ObjDef->StartActions.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[Mission] Running %d Start Actions for %s..."), ObjDef->StartActions.Num(), *ObjectiveID.ToString());
    }

    RunActions(ObjDef->StartActions, Context);

    OnObjectiveStarted.Broadcast(MissionID, ObjectiveID);
}

void UMissionSubsystem::CompleteObjective(FGameplayTag MissionID, FGameplayTag ObjectiveID, bool bSuccess)
{
    // LOG: Entry Point (Helpful to see the start of the frame)
    UE_LOG(LogTemp, Log, TEXT("MissionSubsystem:  Request Complete: %s (Mission: %s) Success: %d"), 
        *ObjectiveID.ToString(), *MissionID.ToString(), bSuccess);

    // --- 1. OPTIMIZATION: Direct Map Lookups ---
    
    // Lookup Mission State ONCE
    FMissionRuntimeState* MissionRt = ActiveMissions.Find(MissionID);
    if (!MissionRt) 
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem:  Failed: Mission %s is not active."), *MissionID.ToString());
        return;
    }

    // Lookup Asset ONCE
    const UMissionData* MissionAsset = GetMissionAsset(MissionID);
    if (!MissionAsset) 
    {
        UE_LOG(LogTemp, Error, TEXT("MissionSubsystem:  Failed: DataAsset for %s missing."), *MissionID.ToString());
        return;
    }

    // Lookup Objective State ONCE
    FObjectiveRuntimeState* ObjRt = MissionRt->ActiveObjectives.Find(ObjectiveID);
    
    // Validate State
    if (!ObjRt)
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem:  Failed: Objective %s is not in ActiveObjectives list."), *ObjectiveID.ToString());
        return;
    }
    if (ObjRt->ObjectiveState != EProgressState::InProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem:  Failed: Objective %s is already %s."), 
            *ObjectiveID.ToString(), 
            (ObjRt->ObjectiveState == EProgressState::Completed ? TEXT("Completed") : TEXT("Failed")));
        return;
    }

    // Find Definition locally
    const UMissionObjective* ObjDef = nullptr;
    for (const auto& Obj : MissionAsset->ObjectiveArray)
    {
        if (Obj && Obj->ObjectiveID == ObjectiveID)
        {
            ObjDef = Obj;
            break;
        }
    }
    if (!ObjDef) 
    {
        UE_LOG(LogTemp, Error, TEXT("MissionSubsystem:  Failed: Objective Definition %s not found in Asset."), *ObjectiveID.ToString());
        return;
    }

    // --- 2. Update State ---

    // Mark Complete
    ObjRt->ObjectiveState = bSuccess ? EProgressState::Completed : EProgressState::Failed;
    
    // Update Mission History
    MissionRt->CompletedObjectiveIDs.Add(ObjectiveID);
    MissionRt->ActiveObjectives.Remove(ObjectiveID);

    // LOG: Vital State Change (Using Display so it's White/Visible in logs)
    UE_LOG(LogTemp, Display, TEXT("MissionSubsystem:  SUCCESS: %s set to %s."), 
        *ObjectiveID.ToString(), (bSuccess ? TEXT("Completed") : TEXT("Failed")));

    // Broadcast
    OnObjectiveCompleted.Broadcast(MissionID, ObjectiveID, bSuccess);

    // --- 3. THE OnComplete LOOP ---
    if (bSuccess)
    {
        TArray<FGameplayTag> ActiveIDs;
        MissionRt->ActiveObjectives.GetKeys(ActiveIDs);

        for (const FGameplayTag& SearchID : ActiveIDs)
        {
            if (SearchID == ObjectiveID) continue; 

            FObjectiveRuntimeState* OtherObjRt = MissionRt->ActiveObjectives.Find(SearchID);
            const UMissionObjective* OtherObjDef = GetObjectiveFromAsset(MissionID, SearchID);

            if (OtherObjRt && OtherObjDef)
            {
                // "Hey Gatekeeper, ObjectiveID just finished."
                if (OtherObjDef->OnObjectiveCompleted(ObjectiveID, *OtherObjRt))
                {
                    // Did that finish the Gatekeeper?
                    if (OtherObjDef->IsComplete(*OtherObjRt))
                    {
                        // LOG: Critical - Identifying Chain Reactions
                        UE_LOG(LogTemp, Display, TEXT("MissionSubsystem:  CHAIN REACTION: %s triggered completion of %s."), 
                            *ObjectiveID.ToString(), *SearchID.ToString());

                        // RECURSION: Finish the Gatekeeper immediately
                        CompleteObjective(MissionID, SearchID, true);
                        
                        if (!ActiveMissions.Contains(MissionID)) return;
                    }
                }
            }
        }
    }

    // --- 4. Handle Flow (Actions & Next Objectives) ---
    
    if (bSuccess)
    {
        // LOG: Flow confirmation
        UE_LOG(LogTemp, Log, TEXT("MissionSubsystem:  Processing Actions/Next Objectives for %s..."), *ObjectiveID.ToString());

        AActor* Context = GetGameInstance()->GetFirstLocalPlayerController()->GetPawn();
        RunActions(ObjDef->CompleteActions, Context);
        ActivateNextObjectives(MissionID, ObjDef->NextObjectiveIDs);
    }

    // --- 5. Check Mission Completion ---
    
    if (MissionRt->ActiveObjectives.Num() > 0)
    {
        return; 
    }

    bool bAllCompleted = true;
    for (const auto& Obj : MissionAsset->ObjectiveArray)
    {
        if (Obj && !MissionRt->CompletedObjectiveIDs.Contains(Obj->ObjectiveID))
        {
            bAllCompleted = false;
            break;
        }
    }

    if (bAllCompleted)
    {
        // LOG: Vital - Mission Finish
        UE_LOG(LogTemp, Display, TEXT("MissionSubsystem:  MISSION COMPLETE: %s has no remaining objectives."), *MissionID.ToString());
        FinishMission(MissionID, true);
    }
}

void UMissionSubsystem::ActivateNextObjectives(FGameplayTag MissionID, const TArray<FGameplayTag>& NextObjectiveIDs)
{
    for (const FGameplayTag& NextObj : NextObjectiveIDs)
    {
        ActivateObjective(MissionID, NextObj);
    }
}

bool UMissionSubsystem::IsObjectiveActive(FGameplayTag MissionID, FGameplayTag ObjectiveID) const
{
	const FObjectiveRuntimeState* ObjRt = GetObjectiveRuntime(MissionID, ObjectiveID);
	return ObjRt && ObjRt->ObjectiveState == EProgressState::InProgress;
}


const UMissionObjective* UMissionSubsystem::GetObjectiveFromAsset(FGameplayTag MissionID, FGameplayTag ObjectiveID) const
{
    const UMissionData* Mission = GetMissionAsset(MissionID);
    if (!Mission) return nullptr;

   const UMissionObjective* ObjDef = nullptr;
    for (const UMissionObjective* Obj : Mission->ObjectiveArray)
    {
        if (Obj && Obj->ObjectiveID == ObjectiveID)
        {
            ObjDef = Obj;
            break;
        }
    }

    if (!ObjDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem:  ActivateObjective: Obj not found: %s"), *ObjectiveID.ToString());
        return nullptr;
    }
    return ObjDef;
}
// ---------- Asset Management ----------

const UMissionData* UMissionSubsystem::GetMissionAsset(FGameplayTag MissionID) const
{
    if (!MissionID.IsValid()) return nullptr;

    // 1. Check cache 
    if (const UMissionData* const* Found = LoadedMissionAssets.Find(MissionID))
    {
        return *Found;
    }

    // 2. Check Asset Manager Memory
    FPrimaryAssetId AssetId(UMissionData::StaticClass()->GetFName(), MissionID.GetTagName());
    UAssetManager& Manager = UAssetManager::Get();
    
    // GetPrimaryAssetObject only returns valid if it's already in RAM.
    return Cast<UMissionData>(Manager.GetPrimaryAssetObject(AssetId));

}


void UMissionSubsystem::OnMissionAssetLoaded(FPrimaryAssetId LoadedId, FGameplayTag MissionID)
{
	
    UAssetManager& Manager = UAssetManager::Get();
    UMissionData* MissionAsset = Cast<UMissionData>(Manager.GetPrimaryAssetObject(LoadedId));

    if (!MissionAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("MissionSubsystem: StartMission: Failed to load asset for %s"), *MissionID.ToString());
        return;
    }

    // Cache it explicitly so it doesn't get garbage collected while active
    LoadedMissionAssets.Add(MissionID, MissionAsset);
    UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: StartMission: Asset loaded. Starting logic for %s"), *MissionID.ToString());

	//Initiate Runtime State
	FMissionRuntimeState& MissionRt = ActiveMissions.FindOrAdd(MissionID);
	MissionRt.MissionID = MissionID;
	MissionRt.MissionState = EProgressState::InProgress;
	OnMissionStarted.Broadcast(MissionID);

	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: StartMission: Mission started: %s"), *MissionID.ToString());

	for (const UMissionObjective* Obj : MissionAsset->ObjectiveArray)
    {
        if (Obj && Obj->bStartAutomatically) // Access property directly
        {
            ActivateObjective(MissionID, Obj->ObjectiveID); 
        }
    }

}

void UMissionSubsystem::ResetSystem()
{
    ActiveMissions.Empty();
    CompletedMissions.Empty();
}

// ---------- Event Bus ----------

void UMissionSubsystem::EmitActorEvent(AActor* SourceActor, FGameplayTag EventTag)
{
    if (!EventTag.IsValid()) return;

    OnMissionEventBroadcast.Broadcast(EventTag);

    // Broadcaster: Send to all active objectives
    for (TPair<FGameplayTag, FMissionRuntimeState>& MissionPair : ActiveMissions)
    {
        const UMissionData* MissionAsset = GetMissionAsset(MissionPair.Key);
        if (!MissionAsset) continue;

        for (const auto& Obj : MissionAsset->ObjectiveArray)
        {
            if (!Obj) continue;

            FObjectiveRuntimeState* ObjRt = MissionPair.Value.ActiveObjectives.Find(Obj->ObjectiveID);
            if (!ObjRt || ObjRt->ObjectiveState != EProgressState::InProgress) continue;

            // Pass to Router
            ConsumeEventForObjective(MissionPair.Key, Obj, *ObjRt, EventTag, SourceActor);
        }
    }
}

void UMissionSubsystem::ConsumeEventForObjective(FGameplayTag MissionID, const UMissionObjective* ObjDef, FObjectiveRuntimeState& ObjRt, FGameplayTag EventTag, AActor* SourceActor)
{
    UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Attempt to Consume Event For Objective"));

    if (ObjDef->OnEvent(MissionID, EventTag, SourceActor, ObjRt))
    {
        UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Consuming Event For Objective"));
        // "Are you done?"
        if (ObjDef->IsComplete(ObjRt))
        {
            CompleteObjective(MissionID, ObjDef->ObjectiveID, true);
        }
    }
}



// ---------- Runtime getters ----------

FMissionRuntimeState* UMissionSubsystem::GetActiveMissionRuntime(FGameplayTag MissionID)
{
	return MissionID.IsValid() ? ActiveMissions.Find(MissionID) : nullptr;
}

const FMissionRuntimeState* UMissionSubsystem::GetActiveMissionRuntime(FGameplayTag MissionID) const
{
	return MissionID.IsValid() ? ActiveMissions.Find(MissionID) : nullptr;
}

FObjectiveRuntimeState* UMissionSubsystem::GetObjectiveRuntime(FGameplayTag MissionID, FGameplayTag ObjectiveID)
{
	FMissionRuntimeState* MissionRt = GetActiveMissionRuntime(MissionID);
	return (MissionRt && ObjectiveID.IsValid()) ? MissionRt->ActiveObjectives.Find(ObjectiveID) : nullptr;
}

const FObjectiveRuntimeState* UMissionSubsystem::GetObjectiveRuntime(FGameplayTag MissionID, FGameplayTag ObjectiveID) const
{
	const FMissionRuntimeState* MissionRt = GetActiveMissionRuntime(MissionID);
	return (MissionRt && ObjectiveID.IsValid()) ? MissionRt->ActiveObjectives.Find(ObjectiveID) : nullptr;
}


// ---------- Actions ----------

void UMissionSubsystem::RunActions(const TArray<TObjectPtr<UMissionAction>>& Actions, AActor* ContextActor)
{
    for (const UMissionAction* Action : Actions)
    {
        if (Action)
        {
            // The Action contains its own logic.
            Action->ExecuteAction(ContextActor);
        }
    }
}


// ---------- Save System ----------
void UMissionSubsystem::SaveToGame(UPeripherySaveGame* SaveObject)
{
    if (!SaveObject) return;

    // Just copy the maps directly. 
    // Unreal handles the struct serialization automatically.
    SaveObject->ActiveMissions = ActiveMissions;
    SaveObject->CompletedMissions = CompletedMissions;
    
    UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Data Saved to Object"));
}

void UMissionSubsystem::LoadFromGame(const UPeripherySaveGame* SaveObject)
{
    if (!SaveObject) return;

    // 1. Clear current state
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    
    // 2. Copy data back
    ActiveMissions = SaveObject->ActiveMissions;
    CompletedMissions = SaveObject->CompletedMissions;

    // 3. CRITICAL: Restore Asset Pointers
    // The SaveFile knows the Tags ("Mission.ShiftStart") but it does NOT store the Asset Pointers.
    // We must reload the assets so the game logic works.
    
    for (auto& Pair : ActiveMissions)
    {
        FGameplayTag MissionID = Pair.Key;
        
        // This function (from our previous steps) forces the asset to load into memory
        // so GetMissionAsset(ID) will work later.
        const UMissionData* Asset = GetMissionAsset(MissionID); 
        
        if (Asset)
        {
            // Explicitly cache it to be safe
            LoadedMissionAssets.Add(MissionID, const_cast<UMissionData*>(Asset));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Data Loaded from Object"));
}
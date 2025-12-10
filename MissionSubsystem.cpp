// MissionSubsystem.cpp

#include "Missions/MissionSubsystem.h"
#include "PeripherySaveGame.h"

#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceNull.h"


// ---------- Initialize ----------

void UMissionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Initialized"));

	FString NetMode = (GetWorld()->GetNetMode() == NM_Client) ? "Client" : "Server";
	UE_LOG(LogTemp, Error, TEXT("MissionSubsystem Init [%s]"), *NetMode);

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
        UE_LOG(LogTemp, Log, TEXT("StartMission: Async loading asset for %s..."), *MissionID.ToString());

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

    //Check if valid section
    if (!IsMissionActive(MissionID)) return;

    const UMissionObjective* ObjDef = GetObjectiveFromAsset(MissionID, ObjectiveID);
    if (!ObjDef) return;


    // Is valid. Start Activate Objective section
    FMissionRuntimeState* MissionRt = GetActiveMissionRuntime(MissionID);
    check(MissionRt);

    FObjectiveRuntimeState& ObjRt = MissionRt->ActiveObjectives.FindOrAdd(ObjectiveID);
    ObjRt.ObjectiveID = ObjectiveID;
    
    // 1. Delegate Initialization to the Object
    // (Sets ProgressState to InProgress, sets up generic storage counters)
    ObjDef->InitializeRuntime(ObjRt);

    // 2. Run Start Actions
    // We pass the PlayerPawn as context (or nullptr if none available)
    AActor* Context = GetGameInstance()->GetFirstLocalPlayerController()->GetPawn();
    RunActions(ObjDef->StartActions, Context);

    OnObjectiveStarted.Broadcast(MissionID, ObjectiveID);
}

void UMissionSubsystem::CompleteObjective(FGameplayTag MissionID, FGameplayTag ObjectiveID, bool bSuccess)
{
    // --- 1. OPTIMIZATION: Direct Map Lookups (Avoid Helper Function Overhead) ---
    
    // Lookup Mission State ONCE
    FMissionRuntimeState* MissionRt = ActiveMissions.Find(MissionID);
    if (!MissionRt) return;

    // Lookup Asset ONCE
    const UMissionData* MissionAsset = GetMissionAsset(MissionID);
    if (!MissionAsset) return;

    // Lookup Objective State ONCE (Directly from the Mission State we just found)
    FObjectiveRuntimeState* ObjRt = MissionRt->ActiveObjectives.Find(ObjectiveID);
    // Validate State
    if (!ObjRt || ObjRt->ObjectiveState != EProgressState::InProgress) return;

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
    if (!ObjDef) return;

    // --- 2. Update State ---

    // Mark Complete
    ObjRt->ObjectiveState = bSuccess ? EProgressState::Completed : EProgressState::Failed;
    
    // Update Mission History
    MissionRt->CompletedObjectiveIDs.Add(ObjectiveID);
    MissionRt->ActiveObjectives.Remove(ObjectiveID);

    // Broadcast (Safe to do now that state is updated)
    OnObjectiveCompleted.Broadcast(MissionID, ObjectiveID, bSuccess);

    // --- 3. THE OnComplete LOOP ---
    // Notify other active objectives that this one finished.
    // We copy the keys because the loop might modify the map (via recursion).
    if (bSuccess)
    {
        TArray<FGameplayTag> ActiveIDs;
        MissionRt->ActiveObjectives.GetKeys(ActiveIDs);

        for (const FGameplayTag& SearchID : ActiveIDs)
        {
            if (SearchID == ObjectiveID) continue; // Don't notify self

            // Fast lookup since we are already in the context of this mission
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
                        // RECURSION: Finish the Gatekeeper immediately
                        CompleteObjective(MissionID, SearchID, true);
                        
                        // Note: MissionRt might be invalid here if the recursion finished the mission!
                        // In a robust system, check if Mission still exists before continuing loop.
                        if (!ActiveMissions.Contains(MissionID)) return;
                    }
                }
            }
        }
    }

    // --- 4. Handle Flow (Actions & Next Objectives) ---
    
    if (bSuccess)
    {
        // Use local player context
        AActor* Context = GetGameInstance()->GetFirstLocalPlayerController()->GetPawn();
        
        // Execute Actions
        RunActions(ObjDef->CompleteActions, Context);

        // Activate Next
        ActivateNextObjectives(MissionID, ObjDef->NextObjectiveIDs);
    }

    // --- 5. Check Mission Completion ---
    
    // OPTIMIZATION: If there are still Active Objectives, the mission CANNOT be done.
    // This saves us from iterating the entire array in 90% of cases.
    if (MissionRt->ActiveObjectives.Num() > 0)
    {
        return; 
    }

    // If active list is empty, we MUST verify against the Asset 
    // (in case there are objectives that haven't started yet).
    bool bAllCompleted = true;
    for (const auto& Obj : MissionAsset->ObjectiveArray)
    {
        // Check if the objective exists AND hasn't been completed yet
        if (Obj && !MissionRt->CompletedObjectiveIDs.Contains(Obj->ObjectiveID))
        {
            bAllCompleted = false;
            break;
        }
    }

    if (bAllCompleted)
    {
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
        UE_LOG(LogTemp, Warning, TEXT("ActivateObjective: Obj not found: %s"), *ObjectiveID.ToString());
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
        UE_LOG(LogTemp, Error, TEXT("StartMission: Failed to load asset for %s"), *MissionID.ToString());
        return;
    }

    // Cache it explicitly so it doesn't get garbage collected while active
    LoadedMissionAssets.Add(MissionID, MissionAsset);
    UE_LOG(LogTemp, Log, TEXT("StartMission: Asset loaded. Starting logic for %s"), *MissionID.ToString());

	//Initiate Runtime State
	FMissionRuntimeState& MissionRt = ActiveMissions.FindOrAdd(MissionID);
	MissionRt.MissionID = MissionID;
	MissionRt.MissionState = EProgressState::InProgress;
	OnMissionStarted.Broadcast(MissionID);

	UE_LOG(LogTemp, Log, TEXT("StartMission: Mission started: %s"), *MissionID.ToString());

	for (const UMissionObjective* Obj : MissionAsset->ObjectiveArray)
    {
        if (Obj && Obj->bStartAutomatically) // Access property directly
        {
            ActivateObjective(MissionID, Obj->ObjectiveID); 
        }
    }

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

    if (ObjDef->OnEvent(MissionID, EventTag, SourceActor, ObjRt))
    {
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
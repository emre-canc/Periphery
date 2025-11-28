// MissionSubsystem.cpp

#include "MissionSubsystem.h"
#include "LevelStateSubsystem.h"
#include "WidgetSubsystem.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

#include "Engine/World.h"
#include "Engine/AssetManager.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
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

	const UMissionData* MissionAsset = GetMissionAsset(MissionID);
    if (!MissionAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: StartMission: Asset not found for: %s"), *MissionID.ToString());
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

EProgressState UMissionSubsystem::GetMissionState(FGameplayTag MissionID) const
{
	const FMissionRuntimeState* Rt = GetActiveMissionRuntime(MissionID);
	return Rt ? Rt->MissionState : EProgressState::NotStarted;
}

FGameplayTag UMissionSubsystem::FindMissionByObjective(FGameplayTag ObjectiveID) const
{
    if (!ObjectiveID.IsValid()) return FGameplayTag();

    for (const TPair<FGameplayTag, FMissionRuntimeState>& Pair : ActiveMissions)
    {
        const UMissionData* MissionAsset = GetMissionAsset(Pair.Key);
        if (!MissionAsset) continue;

        for (const auto& Obj : MissionAsset->ObjectiveArray)
        {
            if (Obj && Obj->ObjectiveID == ObjectiveID)
            {
                return Pair.Key;
            }
        }
    }
    return FGameplayTag();
}

// ---------- Objective control ----------

void UMissionSubsystem::ActivateObjective(FGameplayTag MissionID, FGameplayTag ObjectiveID)
{
    if (!IsMissionActive(MissionID)) return;

    // Get Object Pointer
    const UMissionObjective* ObjDef = GetObjectiveFromAsset(MissionID, ObjectiveID);
    if (!ObjDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("ActivateObjective: Objective not found: %s"), *ObjectiveID.ToString());
        return;
    }

    FMissionRuntimeState* MissionRt = GetActiveMissionRuntime(MissionID);
    check(MissionRt);

    FObjectiveRuntimeState& ObjRt = MissionRt->ActiveObjectives.FindOrAdd(ObjectiveID);
    ObjRt.ObjectiveID = ObjectiveID;
    ObjRt.ObjectiveState = EProgressState::InProgress; // Unified Enum
    ObjRt.CurrentStepIndex = 0;
    ObjRt.bIsActive = true;

    // Initialize per-step runtime map from the UObject->Steps array
    for (const FObjectiveStepDefinition& StepDef : ObjDef->Steps)
    {
        FObjectiveStepRuntimeState StepRt;
        StepRt.StepID = StepDef.StepID;
        ObjRt.PerStepProgress.Add(StepDef.StepID, StepRt);
    }

    // Run Actions
    RunActions(ObjDef->StartActions);

    // If ordered, enter first step
    if (ObjDef->CompletionMode == EObjectivesCompletionMode::CompleteStepsInOrder && ObjDef->Steps.Num() > 0)
    {
        EnterOrderedObjectiveStep(MissionID, ObjectiveID);
    }

    OnObjectiveStarted.Broadcast(MissionID, ObjectiveID);
		UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Objective started: %s"), *ObjectiveID.ToString());
}

void UMissionSubsystem::CompleteObjective(FGameplayTag MissionID, FGameplayTag ObjectiveID, bool bSuccess)
{
    FMissionRuntimeState* MissionRt = GetActiveMissionRuntime(MissionID);
    if (!MissionRt) return;

    FObjectiveRuntimeState* ObjRt = GetObjectiveRuntime(MissionID, ObjectiveID);
    if (!ObjRt || ObjRt->ObjectiveState != EProgressState::InProgress) return;

    const UMissionObjective* ObjDef = GetObjectiveFromAsset(MissionID, ObjectiveID);
    if (!ObjDef) return;

    ObjRt->ObjectiveState = bSuccess ? EProgressState::Completed : EProgressState::Failed;

    OnObjectiveCompleted.Broadcast(MissionID, ObjectiveID, bSuccess);

    MissionRt->CompletedObjectiveIDs.Add(ObjectiveID);
    MissionRt->ActiveObjectives.Remove(ObjectiveID);

    // Copy actions to local scope safely
    const auto ActionsToRun = ObjDef->CompleteActions;
    const auto NextIDs = ObjDef->NextObjectiveIDs;

    GetWorld()->GetTimerManager().SetTimerForNextTick([this, ActionsToRun, MissionID, NextIDs, bSuccess]()
    {
        RunActions(ActionsToRun);

        if (bSuccess && NextIDs.Num() > 0)
        {
            ActivateNextObjectives(MissionID, NextIDs);
        }
    });

    // Check if Mission is Complete (All objectives done?)
    const UMissionData* MissionAsset = GetMissionAsset(MissionID);
    if (MissionAsset)
    {
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
            FinishMission(MissionID, true);
        }
    }
}

bool UMissionSubsystem::IsObjectiveActive(FGameplayTag MissionID, FGameplayTag ObjectiveID) const
{
	const FObjectiveRuntimeState* ObjRt = GetObjectiveRuntime(MissionID, ObjectiveID);
	return ObjRt && ObjRt->ObjectiveState == EProgressState::InProgress && ObjRt->bIsActive;
}

EProgressState UMissionSubsystem::GetObjectiveState(FGameplayTag MissionID, FGameplayTag ObjectiveID) const
{
	const FObjectiveRuntimeState* ObjRt = GetObjectiveRuntime(MissionID, ObjectiveID);
	return ObjRt ? ObjRt->ObjectiveState : EProgressState::NotStarted;
}

// ---------- Event bus ----------

void UMissionSubsystem::EmitActorEvent(AActor* SourceActor, FGameplayTag EventTag)
{
	if (!EventTag.IsValid() || !IsValid(SourceActor))
	{
	 	UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: Tag %s or Actor not valid"), *EventTag.ToString());
	 	return;
	}

	OnMissionEventBroadcast.Broadcast(EventTag);
	bool FoundObjective = false;

for (TPair<FGameplayTag, FMissionRuntimeState>& MissionPair : ActiveMissions)
    {
        const UMissionData* MissionAsset = GetMissionAsset(MissionPair.Key);
        if (!MissionAsset) continue;

        // Loop through active objectives in this mission
        for (const auto& Obj : MissionAsset->ObjectiveArray)
        {
            if (!Obj) continue;

            FObjectiveRuntimeState* ObjRt = MissionPair.Value.ActiveObjectives.Find(Obj->ObjectiveID);
            if (!ObjRt || ObjRt->ObjectiveState != EProgressState::InProgress) continue;

            // Pass the UObject pointer to ConsumeEvent
            ConsumeEventForObjective(MissionPair.Key, Obj, *ObjRt, EventTag, SourceActor);
			FoundObjective = true;
        }
	}
	if (!FoundObjective) UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: Tag %s or Actor not found"), *EventTag.ToString());
}

// ---------- Actor registry ----------

void UMissionSubsystem::RegisterActorForTag(AActor* Actor, FGameplayTag Tag)
{

	// --- Validate input ---
	if (!IsValid(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem::RegisterActorForTag -> Invalid Actor (nullptr or pending kill). Tag: %s"),
			*Tag.ToString());
		return;
	}

	if (!Tag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem::RegisterActorForTag -> Invalid GameplayTag for Actor: %s"),
			*GetNameSafe(Actor));
		return;
	}

	// --- Register actor ---
	TSet<TWeakObjectPtr<AActor>>& SetRef = TagToActors.FindOrAdd(Tag);

	const int32 Before = SetRef.Num();
	SetRef.Add(TWeakObjectPtr<AActor>(Actor)); // safe to call repeatedly
	const bool bAdded = (SetRef.Num() > Before);

	if (bAdded)
	{
		UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Registered Actor '%s' under Tag '%s'"),
			*GetNameSafe(Actor), *Tag.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Actor '%s' was already registered under Tag '%s'"),
			*GetNameSafe(Actor), *Tag.ToString());
	}
}

void UMissionSubsystem::UnregisterActorForTag(AActor* Actor, FGameplayTag Tag)
{
	if (!IsValid(Actor) || !Tag.IsValid()) return;

	if (TSet<TWeakObjectPtr<AActor>>* SetPtr = TagToActors.Find(Tag))
	{
		SetPtr->Remove(TWeakObjectPtr<AActor>(Actor));
		for (auto It = SetPtr->CreateIterator(); It; ++It)
		{
			AActor* Ptr = It->Get();
			if (!IsValid(Ptr)) It.RemoveCurrent();
		}
		if (SetPtr->Num() == 0)
		{
			TagToActors.Remove(Tag);
		}
	}
}

void UMissionSubsystem::RegisterSequenceForTag(AActor* Actor, FGameplayTag Tag)
{
	if (!IsValid(Actor) || !Tag.IsValid()) return;

	TWeakObjectPtr<AActor> WeakActor = Actor;
	TagToSequence.Add(Tag, WeakActor);

	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Registered Sequence '%s' under Tag '%s'"),
		*GetNameSafe(Actor), *Tag.ToString());
}

void UMissionSubsystem::UnregisterSequenceForTag(AActor* Actor, FGameplayTag Tag)
{
	if (!IsValid(Actor) || !Tag.IsValid()) return;

	if (TWeakObjectPtr<AActor>* FoundActor = TagToSequence.Find(Tag))
	{
		if (FoundActor->Get() == Actor)
		{
			TagToSequence.Remove(Tag);
			UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Unregistered Sequence '%s' under Tag '%s'"),
				*GetNameSafe(Actor), *Tag.ToString());
		}
	}
}
	


TArray<AActor*> UMissionSubsystem::GetActorsForTag(FGameplayTag Tag) const
{
	TArray<AActor*> Out;
	if (!Tag.IsValid()) return Out;

	if (const TSet<TWeakObjectPtr<AActor>>* SetPtr = TagToActors.Find(Tag))
	{
		for (const TWeakObjectPtr<AActor>& ObjPtr : *SetPtr)
		{
			if (AActor* A = ObjPtr.Get())
			{
				Out.Add(A);
			}
		}
	}
	return Out;
}

AActor* UMissionSubsystem::GetSequenceForTag(FGameplayTag Tag) const
{
	AActor* ActorOutput = nullptr;
	if (!Tag.IsValid()) return ActorOutput;

	if (const TWeakObjectPtr<AActor>* Found = TagToSequence.Find(Tag))
	{
		ActorOutput = Found->Get(); // get the actual actor
	}

	return ActorOutput;
}

void UMissionSubsystem::PruneTag(FGameplayTag Tag)
{
	if (TSet<TWeakObjectPtr<AActor>>* SetPtr = TagToActors.Find(Tag))
	{
		for (auto It = SetPtr->CreateIterator(); It; ++It)
		{
			AActor* Ptr = It->Get();
			if (!IsValid(Ptr))
			{
				It.RemoveCurrent();
			}
		}
		if (SetPtr->Num() == 0)
		{
			TagToActors.Remove(Tag);
		}
	}
}

// ---------- Data lookup ----------


const UMissionData* UMissionSubsystem::GetMissionAsset(FGameplayTag MissionID) const
{
    if (!MissionID.IsValid()) return nullptr;

    // 1. Check if we already cached it (Optional, but good for speed)
    if (const UMissionData* const* Found = LoadedMissionAssets.Find(MissionID))
    {
        return *Found;
    }

    // 2. Ask Asset Manager
    // This assumes you set up Asset Manager to scan for "MissionData" type
    FPrimaryAssetId AssetId(UMissionData::StaticClass()->GetFName(), MissionID.GetTagName());
    UAssetManager& Manager = UAssetManager::Get();
    
    // NOTE: This returns nullptr if the asset isn't loaded! 
    // In a real game, you should async load missions. For now, we force a sync load (hitchy but works).
    if (UMissionData* LoadedAsset = Cast<UMissionData>(Manager.GetPrimaryAssetObject(AssetId)))
    {
        return LoadedAsset;
    }
    
    // Fallback: Force load (only if Asset Manager knows about it but hasn't loaded it)
    // In production, avoid synchronous loading like this.
    FSoftObjectPath AssetPath = Manager.GetPrimaryAssetPath(AssetId);
    if (AssetPath.IsValid())
    {
        return Cast<UMissionData>(AssetPath.TryLoad());
    }

    return nullptr;
}

const UMissionObjective* UMissionSubsystem::GetObjectiveFromAsset(FGameplayTag MissionID, FGameplayTag ObjectiveID) const
{
    const UMissionData* Mission = GetMissionAsset(MissionID);
    if (!Mission) return nullptr;

    for (const UMissionObjective* Obj : Mission->ObjectiveArray)
    {
        if (Obj && Obj->ObjectiveID == ObjectiveID)
        {
            return Obj;
        }
    }
    return nullptr;
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

// ---------- Step flow helpers ----------
void UMissionSubsystem::EnterOrderedObjectiveStep(FGameplayTag MissionID, FGameplayTag ObjectiveID)
{
    const UMissionObjective* ObjDef = GetObjectiveFromAsset(MissionID, ObjectiveID);
    FObjectiveRuntimeState* ObjRt = GetObjectiveRuntime(MissionID, ObjectiveID);
    if (!ObjDef || !ObjRt) return;

    // Use ->Steps
    if (!ObjDef->Steps.IsValidIndex(ObjRt->CurrentStepIndex)) return;

    const FObjectiveStepDefinition& StepDef = ObjDef->Steps[ObjRt->CurrentStepIndex];
    RunActions(StepDef.StepBeginActions);
}

void UMissionSubsystem::OnStepCompleted(FGameplayTag MissionID, FGameplayTag ObjectiveID, const FName& StepID)
{
    const UMissionObjective* ObjDef = GetObjectiveFromAsset(MissionID, ObjectiveID);
    FObjectiveRuntimeState* ObjRt = GetObjectiveRuntime(MissionID, ObjectiveID);
    if (!ObjDef || !ObjRt) return;

    if (FObjectiveStepRuntimeState* StepRt = ObjRt->PerStepProgress.Find(StepID))
    {
        StepRt->bStepCompleted = true;
    }

    // Use ->Steps
    const FObjectiveStepDefinition* StepDefPtr = ObjDef->Steps.FindByPredicate(
        [&](const FObjectiveStepDefinition& S){ return S.StepID == StepID; });
        
    if (StepDefPtr)
    {
        RunActions(StepDefPtr->StepCompleteActions);
    }

    switch (ObjRt->CompletionMode)
    {
    case EObjectivesCompletionMode::CompleteStepsInOrder:
        {
            ObjRt->CurrentStepIndex++;
            // Use ->Steps.Num()
            if (ObjRt->CurrentStepIndex >= ObjDef->Steps.Num())
            {
                CompleteObjective(MissionID, ObjectiveID, true);
            }
            else
            {
                EnterOrderedObjectiveStep(MissionID, ObjectiveID);
            }
            break;
        }
    case EObjectivesCompletionMode::CompleteAllSteps:
        {
            if (IsObjectiveCompleted_All(ObjDef, *ObjRt))
            {
                CompleteObjective(MissionID, ObjectiveID, true);
            }
            break;
        }
    case EObjectivesCompletionMode::CompleteAnySingleStep:
        {
            CompleteObjective(MissionID, ObjectiveID, true);
            break;
        }
    default: break;
    }
}

void UMissionSubsystem::ConsumeEventForObjective(FGameplayTag MissionID, const UMissionObjective* ObjDef, FObjectiveRuntimeState& ObjRt, FGameplayTag EventTag, AActor* SourceActor)
{
	if (ObjRt.ObjectiveState != EProgressState::InProgress) return;
	if (!ObjDef) return;

	auto ConsumeForStep = [&](const FObjectiveStepDefinition& StepDef)
	{
		FObjectiveStepRuntimeState* StepRt = ObjRt.PerStepProgress.Find(StepDef.StepID);
		if (!StepRt || StepRt->bStepCompleted) return false;

		bool bMatchedAny = false;

		// Increment counts for matching requirements
		for (const FStepRequirement& Req : StepDef.RequiredEventsToCompleteStep)
		{
			if (EventTag.MatchesTagExact(Req.RequiredEventTag))
			{
				if (!Req.RequireUniqueSources || (IsValid(SourceActor)) && !StepRt->CountedSources.Contains(SourceActor)) {
					int32& Seen = StepRt->SeenEventCounts.FindOrAdd(Req.RequiredEventTag);
					Seen++;
					bMatchedAny = true;
					StepRt->CountedSources.Add(SourceActor);
					UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Event consumed: %s"), *EventTag.ToString());
				} else {
					UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: Unique actor event already consumed: %s"), *EventTag.ToString());
				}
			}
		}

		if (!bMatchedAny) 
		{
			UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: Event not consumed: %s"), *EventTag.ToString());
			return false;
		}

		// Check if all requirements are met
		bool bAllMet = true;
		for (const FStepRequirement& Req : StepDef.RequiredEventsToCompleteStep)
		{
			const int32* SeenPtr = StepRt->SeenEventCounts.Find(Req.RequiredEventTag);
			const int32 Seen = SeenPtr ? *SeenPtr : 0;
			if (Seen < Req.NumberOfTimesEventMustOccur)
			{
				UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Event consumption count: %i"), Seen)
				bAllMet = false;
				break;
			}
		}

		if (bAllMet)
		{
			OnStepCompleted(MissionID, ObjRt.ObjectiveID, StepDef.StepID);
			return true;
		}

		return false;
	};

	switch (ObjRt.CompletionMode)
	{
	case EObjectivesCompletionMode::CompleteStepsInOrder:
        {
            if (ObjDef->Steps.IsValidIndex(ObjRt.CurrentStepIndex))
            {
                ConsumeForStep(ObjDef->Steps[ObjRt.CurrentStepIndex]);
            }
            break;
        }
	case EObjectivesCompletionMode::CompleteAllSteps:
		{
			for (const FObjectiveStepDefinition& StepDef : ObjDef->Steps)
			{
				ConsumeForStep(StepDef); // keep looping; multiple steps might complete due to same event (rare)
			}
			break;
		}
	case EObjectivesCompletionMode::CompleteAnySingleStep:
		{
			for (const FObjectiveStepDefinition& StepDef : ObjDef->Steps)
			{
				if (ConsumeForStep(StepDef))
				{
					// This will complete the objective; we can early-out
					break;
				}
			}
			break;
		}
	default:
		break;
	}
}

// ---------- Actions ----------

void UMissionSubsystem::RunActions(const TArray<FObjectiveActionDefinition>& Actions)
{
	for (const FObjectiveActionDefinition& A : Actions)
	{
		RunAction(A);
	}
}

void UMissionSubsystem::RunAction(const FObjectiveActionDefinition& Action)
{
	switch (Action.ActionType)
	{
	case EObjectiveActionType::ApplyLevelEnvironmentPhase:
		ApplyLevelEnvironmentPhase(Action.LevelEnvironmentPhaseChannelName, Action.LevelEnvironmentPhaseName);
		break;

	case EObjectiveActionType::SendCommandToActor:
		SendCommandToActor(Action.ActorTagToReceiveCommand, Action.CommandNameToSend);
		break;

	case EObjectiveActionType::ShowOrHideWidget:
		ShowOrHideWidget(Action.WidgetTag);
		break;

	case EObjectiveActionType::PlayOrStopSequence:
		PlayOrStopSequence(Action.SequenceTag);
		break;

	default:
		break;
	}
	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: RunAction: %s"), *UEnum::GetValueAsString(Action.ActionType));
}

void UMissionSubsystem::SendCommandToActor(FGameplayTag ActorTag, FName CommandName)
{

	if (!ActorTag.IsValid() || CommandName.IsNone()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: SendCommand ERROR"));
		return;
	}

	TArray<AActor*> Actors = GetActorsForTag(ActorTag);
	for (AActor* A : Actors)
	{
		if (!IsValid(A)) 
		{
			UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: HandleCommand cannot find actors with ActorTag"));
			continue;
		}

		// Expect a function named "HandleCommand" taking an FName or similar.
		if (UFunction* Fn = A->FindFunction(TEXT("HandleCommand")))
		{
			struct { FName Command; } Params{ CommandName };
			A->ProcessEvent(Fn, &Params);
			continue;
		}

		// Fallback: Call by name with string arg (optional)
		UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Fallback, looking for HandleCommand by name"));
		FOutputDeviceNull Ar;
		const FString ArgStr = CommandName.ToString();
		A->CallFunctionByNameWithArguments(*FString::Printf(TEXT("HandleCommand %s"), *ArgStr), Ar, nullptr, true);
	}

	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Sent command '%s' to Actor '%s' (%d actors)"),
		*CommandName.ToString(), *ActorTag.ToString(), Actors.Num());
}

void UMissionSubsystem::ApplyLevelEnvironmentPhase(FName ChannelName, FName PhaseName)
{
    //helper function for LevelSubsystem
    if (ULevelStateSubsystem* LevelState = ULevelStateSubsystem::GetLevel(this))
    {
        LevelState->ApplyPhase(ChannelName, PhaseName);
        UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Applied Level Phase | Channel='%s' Phase='%s'"),
            *ChannelName.ToString(), *PhaseName.ToString());
        return;
    }

    // Fallback/log if not available
    UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: ApplyLevelEnvironmentPhase failed (no LevelStateSubsystem). Channel='%s' Phase='%s'"),
        *ChannelName.ToString(), *PhaseName.ToString());
}

void UMissionSubsystem::ShowOrHideWidget(FGameplayTag WidgetTag)
{
	if (!WidgetTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: Invalid WidgetTag"));
		return;
	}

	if (UWidgetSubsystem* WidgetSystem =  GetGameInstance()->GetSubsystem<UWidgetSubsystem>())
	{
		WidgetSystem->ToggleByContext(WidgetTag);
		UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: Toggled widget %s"), *WidgetTag.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: No WidgetSubsystem found"));
	}
}

void UMissionSubsystem::PlayOrStopSequence(FGameplayTag SequenceTag)
{
    if (!SequenceTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: PlayOrStopSequence skipped — invalid SequenceTag"));
        return;
    }

	if (!GetWorld() || GetWorld()->bIsTearingDown) return;

    ALevelSequenceActor* SeqActor = nullptr;

    if (TWeakObjectPtr<AActor>* Found = TagToSequence.Find(SequenceTag))
    {
        SeqActor = Cast<ALevelSequenceActor>(Found->Get());
    }

    if (!IsValid(SeqActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: PlayOrStopSequence — no LevelSequenceActor registered for tag '%s'"),
            *SequenceTag.ToString());
        return;
    }

    if (ULevelSequencePlayer* SeqPlayer = SeqActor->GetSequencePlayer())
    {
		if (SeqPlayer->IsPlaying()) 
		{
			SeqPlayer->Stop();
			UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: PlayOrStopSequence — stopped '%s' (Tag: %s)"),
				*GetNameSafe(SeqActor), *SequenceTag.ToString());
		} else 
		{
			SeqPlayer->Play();
			UE_LOG(LogTemp, Log, TEXT("MissionSubsystem: PlayOrStopSequence — played '%s' (Tag: %s)"),
				*GetNameSafe(SeqActor), *SequenceTag.ToString());
		}
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionSubsystem: PlayOrStopSequence — SequencePlayer missing on '%s' (Tag: %s)"),
            *GetNameSafe(SeqActor), *SequenceTag.ToString());
    }
}







// ---------- Completion helpers ----------

bool UMissionSubsystem::IsObjectiveCompleted_All(const UMissionObjective* ObjDef, const FObjectiveRuntimeState& ObjRt) const
{
    if (!ObjDef) return false;
    for (const FObjectiveStepDefinition& StepDef : ObjDef->Steps)
    {
        const FObjectiveStepRuntimeState* StepRt = ObjRt.PerStepProgress.Find(StepDef.StepID);
        if (!StepRt || !StepRt->bStepCompleted) return false;
    }
    return true;
}

bool UMissionSubsystem::IsObjectiveCompleted_Any(const UMissionObjective* ObjDef, const FObjectiveRuntimeState& ObjRt) const
{
    if (!ObjDef) return false;
    for (const FObjectiveStepDefinition& StepDef : ObjDef->Steps)
    {
        const FObjectiveStepRuntimeState* StepRt = ObjRt.PerStepProgress.Find(StepDef.StepID);
        if (StepRt && StepRt->bStepCompleted) return true;
    }
    return false;
}

void UMissionSubsystem::ActivateNextObjectives(FGameplayTag MissionID, const TArray<FGameplayTag>& NextObjectiveIDs)
{
	for (const FGameplayTag& NextObj : NextObjectiveIDs)
	{
		ActivateObjective(MissionID, NextObj);
	}
}

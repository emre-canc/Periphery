
#include "Subsystems/ActorRegistrySubsystem.h"

#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Misc/OutputDeviceNull.h"


// ---------- Actor registry ----------
void UActorRegistrySubsystem::RegisterActorForTag(AActor* Actor, FGameplayTag Tag)
{

	// --- Validate input ---
	if (!IsValid(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("ActorRegistrySubsystem::RegisterActorForTag -> Invalid Actor (nullptr or pending kill). Tag: %s"),
			*Tag.ToString());
		return;
	}

	if (!Tag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ActorRegistrySubsystem::RegisterActorForTag -> Invalid GameplayTag for Actor: %s"),
			*GetNameSafe(Actor));
		return;
	}

	// Registering actor
	TSet<TWeakObjectPtr<AActor>>& SetRef = TagToActors.FindOrAdd(Tag);

	const int32 Before = SetRef.Num();
	SetRef.Add(TWeakObjectPtr<AActor>(Actor)); // safe to call repeatedly
	const bool bAdded = (SetRef.Num() > Before);

	if (bAdded)
	{
		UE_LOG(LogTemp, Log, TEXT("ActorRegistrySubsystem: Registered Actor '%s' under Tag '%s'"),
			*GetNameSafe(Actor), *Tag.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ActorRegistrySubsystem: Actor '%s' was already registered under Tag '%s'"),
			*GetNameSafe(Actor), *Tag.ToString());
	}
}

void UActorRegistrySubsystem::UnregisterActorForTag(AActor* Actor, FGameplayTag Tag)
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

void UActorRegistrySubsystem::RegisterSequenceForTag(AActor* Actor, FGameplayTag Tag)
{
	if (!IsValid(Actor) || !Tag.IsValid()) return;

	TWeakObjectPtr<AActor> WeakActor = Actor;
	TagToSequence.Add(Tag, WeakActor);

	UE_LOG(LogTemp, Log, TEXT("ActorRegistrySubsystem: Registered Sequence '%s' under Tag '%s'"),
		*GetNameSafe(Actor), *Tag.ToString());
}

void UActorRegistrySubsystem::UnregisterSequenceForTag(AActor* Actor, FGameplayTag Tag)
{
	if (!IsValid(Actor) || !Tag.IsValid()) return;

	if (TWeakObjectPtr<AActor>* FoundActor = TagToSequence.Find(Tag))
	{
		if (FoundActor->Get() == Actor)
		{
			TagToSequence.Remove(Tag);
			UE_LOG(LogTemp, Log, TEXT("ActorRegistrySubsystem: Unregistered Sequence '%s' under Tag '%s'"),
				*GetNameSafe(Actor), *Tag.ToString());
		}
	}
}

TArray<AActor*> UActorRegistrySubsystem::GetActorsForTag(FGameplayTag Tag) const
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

AActor* UActorRegistrySubsystem::GetSequenceForTag(FGameplayTag Tag) const
{
	AActor* ActorOutput = nullptr;
	if (!Tag.IsValid()) return ActorOutput;

	if (const TWeakObjectPtr<AActor>* Found = TagToSequence.Find(Tag))
	{
		ActorOutput = Found->Get(); // get the actual actor
	}

	return ActorOutput;
}



// ---------- Helper ----------
void UActorRegistrySubsystem::PruneTag(FGameplayTag Tag)
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

// ---------- Save System ----------

void UActorRegistrySubsystem::RegisterSaveableActor(AActor* Actor, FGuid ActorGuid)
{
    if (!Actor || !ActorGuid.IsValid())
    {
        return;
    }

    // Add or Update the entry
    GuidToActorMap.Add(ActorGuid, Actor);
}

void UActorRegistrySubsystem::UnregisterSaveableActor(FGuid ActorGuid)
{
    if (ActorGuid.IsValid())
    {
        GuidToActorMap.Remove(ActorGuid);
    }
}

AActor* UActorRegistrySubsystem::GetActorByGuid(FGuid ActorGuid) const
{
    if (const TWeakObjectPtr<AActor>* FoundActorPtr = GuidToActorMap.Find(ActorGuid))
    {
        return FoundActorPtr->Get(); // Returns nullptr if the actor is dead/stale
    }
    return nullptr;
}

TArray<AActor*> UActorRegistrySubsystem::GetAllSaveableActors() const
{
    TArray<AActor*> ValidActors;
    ValidActors.Reserve(GuidToActorMap.Num());

    for (const auto& Pair : GuidToActorMap)
    {
        if (AActor* Actor = Pair.Value.Get())
        {
            ValidActors.Add(Actor);
        }
    }
    return ValidActors;
}



// ---------- Actions ----------


void UActorRegistrySubsystem::SendCommandToActor(FGameplayTag ActorTag, FName CommandName)
{

	if (!ActorTag.IsValid() || CommandName.IsNone()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("ActorRegistrySubsystem: SendCommand ERROR"));
		return;
	}

	TArray<AActor*> Actors = GetActorsForTag(ActorTag);
	for (AActor* A : Actors)
	{
		if (!IsValid(A)) 
		{
			UE_LOG(LogTemp, Warning, TEXT("ActorRegistrySubsystem: HandleCommand cannot find actors with ActorTag"));
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
		UE_LOG(LogTemp, Log, TEXT("ActorRegistrySubsystem: Fallback, looking for HandleCommand by name"));
		FOutputDeviceNull Ar;
		const FString ArgStr = CommandName.ToString();
		A->CallFunctionByNameWithArguments(*FString::Printf(TEXT("HandleCommand %s"), *ArgStr), Ar, nullptr, true);
	}
}






void UActorRegistrySubsystem::PlayOrStopSequence(FGameplayTag SequenceTag)
{
    if (!SequenceTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("ActorRegistrySubsystem: PlayOrStopSequence skipped — invalid SequenceTag"));
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
        UE_LOG(LogTemp, Warning, TEXT("ActorRegistrySubsystem: PlayOrStopSequence — no LevelSequenceActor registered for tag '%s'"),
            *SequenceTag.ToString());
        return;
    }

    if (ULevelSequencePlayer* SeqPlayer = SeqActor->GetSequencePlayer())
    {
		if (SeqPlayer->IsPlaying()) 
		{
			SeqPlayer->Stop();
			UE_LOG(LogTemp, Log, TEXT("ActorRegistrySubsystem: PlayOrStopSequence — stopped '%s' (Tag: %s)"),
				*GetNameSafe(SeqActor), *SequenceTag.ToString());
		} else 
		{
			SeqPlayer->Play();
			UE_LOG(LogTemp, Log, TEXT("ActorRegistrySubsystem: PlayOrStopSequence — played '%s' (Tag: %s)"),
				*GetNameSafe(SeqActor), *SequenceTag.ToString());
		}
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ActorRegistrySubsystem: PlayOrStopSequence — SequencePlayer missing on '%s' (Tag: %s)"),
            *GetNameSafe(SeqActor), *SequenceTag.ToString());
    }
}


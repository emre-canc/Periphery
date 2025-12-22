#include "Subsystems/ActorRegistrySubsystem.h"
#include "Interfaces/ActorRegistryInterface.h"
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

void UActorRegistrySubsystem::RemoveActorFromAllTags(AActor* Actor)
{
    if (!Actor) return;

    // Iterate over the entire map (Values are Sets of Actors)
    for (auto It = TagToActors.CreateIterator(); It; ++It)
    {
        TSet<TWeakObjectPtr<AActor>>& ActorSet = It.Value();
        
        // Remove the actor if present
        if (ActorSet.Remove(Actor) > 0)
        {
            // If the set becomes empty, we can remove the Tag entry entirely
            if (ActorSet.IsEmpty())
            {
                It.RemoveCurrent();
            }
        }
    }
}

// ---------- Queries ----------
TArray<AActor*> UActorRegistrySubsystem::GetActorsForTag(FGameplayTag Tag) const 
{
    TArray<AActor*> Results;

    if (const TSet<TWeakObjectPtr<AActor>>* FoundSet = TagToActors.Find(Tag))
    {
        for (const TWeakObjectPtr<AActor>& WeakActor : *FoundSet)
        {
            if (AActor* LiveActor = WeakActor.Get())
            {
                Results.Add(LiveActor);
            }
        }
    }
    return Results;
}

TArray<AActor*> UActorRegistrySubsystem::GetActorsWithIntersection(FGameplayTag TagA, FGameplayTag TagB)
{
    // 1. Get Set A (using hierarchy search)
    TArray<AActor*> ListA = GetActors(TagA);
    if (ListA.IsEmpty()) return TArray<AActor*>();

    // 2. Get Set B (using hierarchy search)
    TArray<AActor*> ListB = GetActors(TagB);
    if (ListB.IsEmpty()) return TArray<AActor*>();

    // 3. Convert List B to Set for O(1) lookup speed
    TSet<AActor*> SetB(ListB);

    // 4. Filter A
    TArray<AActor*> Intersection;
    for (AActor* Actor : ListA)
    {
        if (SetB.Contains(Actor))
        {
            Intersection.Add(Actor);
        }
    }

    return Intersection;
}

TArray<AActor*> UActorRegistrySubsystem::GetActors(FGameplayTag Tag) const
{
    TArray<AActor*> Results;
    if (!Tag.IsValid()) return Results;

    // Iterate over ALL registered tags to find children
    for (const auto& Pair : TagToActors)
    {
        const FGameplayTag& RegisteredTag = Pair.Key;

        // MatchesTag returns true for Exact Matches AND Child Matches
        if (RegisteredTag.MatchesTag(Tag))
        {
            const TSet<TWeakObjectPtr<AActor>>& ActorSet = Pair.Value;
            
            for (const TWeakObjectPtr<AActor>& WeakActor : ActorSet)
            {
                if (AActor* LiveActor = WeakActor.Get())
                {
                    Results.AddUnique(LiveActor);
                }
            }
        }
    }
    return Results;
}

AActor* UActorRegistrySubsystem::FindActor(FGameplayTag Tag) const
{
    if (const TSet<TWeakObjectPtr<AActor>>* FoundSet = TagToActors.Find(Tag))
    {
        for (const TWeakObjectPtr<AActor>& WeakActor : *FoundSet)
        {
            if (AActor* LiveActor = WeakActor.Get())
            {
                return LiveActor; // Return the first valid one immediately
            }
        }
    }
    return nullptr;
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
        UE_LOG(LogTemp, Warning, TEXT("ActorRegistrySubsystem: SendCommand ERROR - Invalid Tag or Command"));
        return;
    }

    TArray<AActor*> Actors = GetActors(ActorTag);

    for (AActor* A : Actors)
    {
        // 1. Check for Interface Implementation
        if (A && A->Implements<UActorRegistryInterface>())
        {
            // 2. Execute the Interface call safely
            IActorRegistryInterface::Execute_HandleCommand(A, CommandName);
        }
        else if (A)
        {
            UE_LOG(LogTemp, Verbose, TEXT("ActorRegistrySubsystem: Actor %s found for tag %s but does not implement ActorRegistryInterface."), *A->GetName(), *ActorTag.ToString());
        }
    }
}

// void UActorRegistrySubsystem::PlayOrStopSequence(FGameplayTag SequenceTag) // use Commands "PlaySequence" instead

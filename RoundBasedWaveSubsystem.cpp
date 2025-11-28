// RoundBasedWaveSubsystem.cpp

#include "RoundBasedWaveSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	// Spawn delay starts high and shrinks as tokens are spent
	const float MaxSpawnDelay = 3.0f;   // at start of wave
	const float MinSpawnDelay = 0.25f;  // near end of wave
	const float SpawnTickInterval = 0.25f; // how often subsystem updates cooldown
}

URoundBasedWaveSubsystem* URoundBasedWaveSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		return World->GetSubsystem<URoundBasedWaveSubsystem>();
	}
	return nullptr;
}

void URoundBasedWaveSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("Waves: Deinitialize called. Clearing timers and resetting state."));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTickHandle);
	}

	Spawners.Empty();
	bWaveActive            = false;
	CurrentWaveIndex       = -1;
	TokensBudgetThisWave   = 0;
	TokensSpentThisWave    = 0;
	EnemiesAlive           = 0;
	SpawnCooldownRemaining = 0.f;

	Super::Deinitialize();
}

void URoundBasedWaveSubsystem::RegisterSpawner(AActor* Spawner)
{
	if (!Spawner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Waves: RegisterSpawner called with null Spawner."));
		return;
	}

	Spawners.AddUnique(TWeakObjectPtr<AActor>(Spawner));
	UE_LOG(LogTemp, Log, TEXT("Waves: Registered spawner %s. Total spawners: %d"),
		*Spawner->GetName(), Spawners.Num());
}

void URoundBasedWaveSubsystem::UnregisterSpawner(AActor* Spawner)
{
	if (!Spawner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Waves: UnregisterSpawner called with null Spawner."));
		return;
	}

	const int32 Removed = Spawners.RemoveAll(
		[Spawner](const TWeakObjectPtr<AActor>& Weak)
		{
			return Weak.Get() == Spawner;
		}
	);

	UE_LOG(LogTemp, Log, TEXT("Waves: Unregistered spawner %s. Removed %d. Remaining spawners: %d"),
		*Spawner->GetName(), Removed, Spawners.Num());
}

void URoundBasedWaveSubsystem::SetWaveConfig(
	int32 InFirstWaveTokens,
	float InTokensPerWaveAdd,
	float InTokensPerWaveMultiplier,
	int32 InMaxWaveCount,
	const TArray<FEnemyTokenOption>& InEnemyOptions)
{
	FirstWaveTokens         = FMath::Max(0, InFirstWaveTokens);
	TokensPerWaveAdd        = InTokensPerWaveAdd;
	TokensPerWaveMultiplier = InTokensPerWaveMultiplier;
	MaxWaveCount            = FMath::Max(0, InMaxWaveCount);
	EnemyOptions            = InEnemyOptions;

	UE_LOG(LogTemp, Log, TEXT("Waves: SetWaveConfig - First=%d Add=%.2f Mult=%.3f MaxWaveCount=%d EnemyOptions=%d"),
		FirstWaveTokens,
		TokensPerWaveAdd,
		TokensPerWaveMultiplier,
		MaxWaveCount,
		EnemyOptions.Num());
}

void URoundBasedWaveSubsystem::BeginNextWave()
{
	UE_LOG(LogTemp, Log, TEXT("Waves: BeginNextWave called. CurrentWaveIndex=%d bWaveActive=%d"),
		CurrentWaveIndex, bWaveActive ? 1 : 0);

	if (bWaveActive) return;

	const int32 NextWaveIndex = CurrentWaveIndex + 1;

	// MaxWaveCount == 0 => infinite
	if (MaxWaveCount > 0 && NextWaveIndex >= MaxWaveCount)
	{
		UE_LOG(LogTemp, Log, TEXT("Waves: BeginNextWave aborted. NextWaveIndex=%d >= MaxWaveCount=%d"),
			NextWaveIndex, MaxWaveCount);
		return; // finished all waves
	}

	BeginWaveInternal(NextWaveIndex);
}

void URoundBasedWaveSubsystem::ForceEndWave(bool /*bKillRemaining*/)
{
	UE_LOG(LogTemp, Log, TEXT("Waves: ForceEndWave called. bWaveActive=%d"), bWaveActive ? 1 : 0);

	if (!bWaveActive) return;
	EndCurrentWave();
}

void URoundBasedWaveSubsystem::NotifyEnemySpawned(APawn* Enemy)
{
	if (!bWaveActive || !Enemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("Waves: NotifyEnemySpawned ignored. bWaveActive=%d Enemy=%s"),
			bWaveActive ? 1 : 0,
			Enemy ? *Enemy->GetName() : TEXT("nullptr"));
		return;
	}

	++EnemiesAlive;

	UClass* SpawnedClass = Enemy->GetClass();
	if (!SpawnedClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Waves: NotifyEnemySpawned - Enemy %s has null class."),
			*Enemy->GetName());
		return;
	}

	int32 FoundCost = 0;
	const FString SpawnedClassName = SpawnedClass->GetName();

	for (const FEnemyTokenOption& Opt : EnemyOptions)
	{
		if (!Opt.EnemyClass) continue;

		if (SpawnedClass->IsChildOf(Opt.EnemyClass))
		{
			FoundCost = Opt.TokenCost;
			break;
		}
	}

	if (FoundCost <= 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Waves: NotifyEnemySpawned - Enemy %s (Class=%s) had NO matching token cost. TokensSpent=%d / Budget=%d"),
			*Enemy->GetName(),
			*SpawnedClassName,
			TokensSpentThisWave,
			TokensBudgetThisWave);
		return;
	}

	const int32 OldSpent = TokensSpentThisWave;

	TokensSpentThisWave = FMath::Clamp(
		TokensSpentThisWave + FoundCost,
		0,
		TokensBudgetThisWave
	);

	UE_LOG(LogTemp, Log,
		TEXT("Waves: NotifyEnemySpawned - Enemy=%s Class=%s Cost=%d OldSpent=%d NewSpent=%d Remaining=%d EnemiesAlive=%d"),
		*Enemy->GetName(),
		*SpawnedClassName,
		FoundCost,
		OldSpent,
		TokensSpentThisWave,
		GetTokensRemaining(),
		EnemiesAlive);
}

void URoundBasedWaveSubsystem::NotifyEnemyDied(APawn* Enemy)
{
	if (!bWaveActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Waves: NotifyEnemyDied called while wave inactive. Enemy=%s"),
			Enemy ? *Enemy->GetName() : TEXT("nullptr"));
		return;
	}

	if (EnemiesAlive > 0)
	{
		--EnemiesAlive;
	}

	UE_LOG(LogTemp, Log,
		TEXT("Waves: NotifyEnemyDied - Enemy=%s EnemiesAlive=%d TokensRemaining=%d"),
		Enemy ? *Enemy->GetName() : TEXT("nullptr"),
		EnemiesAlive,
		GetTokensRemaining());

	if (GetTokensRemaining() <= 0 && EnemiesAlive <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Waves: All tokens spent and all enemies dead. Ending wave %d."),
			CurrentWaveIndex);
		EndCurrentWave();
	}
}

void URoundBasedWaveSubsystem::BeginWaveInternal(int32 WaveIndex)
{
	CurrentWaveIndex     = WaveIndex;
	TokensBudgetThisWave = ComputeTokenBudgetForWave(WaveIndex);
	TokensSpentThisWave  = 0;
	EnemiesAlive         = 0;
	bWaveActive          = true;

	// Start with a long delay (slow spawning at wave start)
	SpawnCooldownRemaining = MaxSpawnDelay;

	UE_LOG(LogTemp, Log,
		TEXT("Waves: BeginWaveInternal - WaveIndex=%d Budget=%d CooldownStart=%.2f Spawners=%d"),
		CurrentWaveIndex,
		TokensBudgetThisWave,
		SpawnCooldownRemaining,
		Spawners.Num());

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SpawnTickHandle,
			this,
			&URoundBasedWaveSubsystem::SpawnTick,
			SpawnTickInterval,
			true
		);
	}

	OnWaveStarted.Broadcast(CurrentWaveIndex);
}

void URoundBasedWaveSubsystem::EndCurrentWave()
{
	if (!bWaveActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Waves: EndCurrentWave called while wave already inactive."));
		return;
	}

	UE_LOG(LogTemp, Log,
		TEXT("Waves: EndCurrentWave - WaveIndex=%d TokensSpent=%d / %d EnemiesAlive=%d"),
		CurrentWaveIndex,
		TokensSpentThisWave,
		TokensBudgetThisWave,
		EnemiesAlive);

	bWaveActive = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTickHandle);
	}

	OnWaveFinished.Broadcast(CurrentWaveIndex);
}

void URoundBasedWaveSubsystem::SpawnTick()
{
	if (!bWaveActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Waves: SpawnTick called while wave inactive. Ignoring."));
		return;
	}

	const int32 TokensRemaining = GetTokensRemaining();

	// No more tokens to spend
	if (TokensRemaining <= 0)
	{
		UE_LOG(LogTemp, Log,
			TEXT("Waves: SpawnTick - No tokens remaining. EnemiesAlive=%d"),
			EnemiesAlive);

		// If everything is dead, end; else stop spawning and just wait
		if (EnemiesAlive <= 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Waves: SpawnTick - Ending wave because no enemies alive."));
			EndCurrentWave();
		}
		else if (UWorld* World = GetWorld())
		{
			UE_LOG(LogTemp, Log, TEXT("Waves: SpawnTick - Stopping spawn tick. Waiting for enemies to die."));
			World->GetTimerManager().ClearTimer(SpawnTickHandle);
		}
		return;
	}

	// Cooldown logic: don’t spawn every tick
	SpawnCooldownRemaining -= SpawnTickInterval;
	if (SpawnCooldownRemaining > 0.f)
	{
		// Optional: very light log if needed
		// UE_LOG(LogTemp, Verbose, TEXT("Waves: SpawnTick - Cooldown=%.2f"), SpawnCooldownRemaining);
		return;
	}

	// Time to schedule a spawn
	UE_LOG(LogTemp, Log,
		TEXT("Waves: SpawnTick - Attempting spawn. Wave=%d TokensRemaining=%d EnemiesAlive=%d"),
		CurrentWaveIndex,
		TokensRemaining,
		EnemiesAlive);

	if (TrySpawnFromTokens())
	{
		// Recompute delay based on how far into the wave we are
		const float Progress = (TokensBudgetThisWave > 0)
			? FMath::Clamp(
				static_cast<float>(TokensSpentThisWave) /
				static_cast<float>(TokensBudgetThisWave),
				0.f, 1.f)
			: 1.f;

		const float CurrentDelay = FMath::Lerp(
			MaxSpawnDelay,
			MinSpawnDelay,
			Progress
		);

		SpawnCooldownRemaining = CurrentDelay;

		UE_LOG(LogTemp, Log,
			TEXT("Waves: SpawnTick - Spawn requested. TokensSpent=%d / %d Progress=%.2f NewCooldown=%.2f"),
			TokensSpentThisWave,
			TokensBudgetThisWave,
			Progress,
			SpawnCooldownRemaining);
	}
	else
	{
		UE_LOG(LogTemp, Log,
			TEXT("Waves: SpawnTick - TrySpawnFromTokens failed (no affordable or no spawners). Stopping spawn tick."));

		// No affordable enemies left; stop spawning and wait for wave to end
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(SpawnTickHandle);
		}
	}
}

int32 URoundBasedWaveSubsystem::ComputeTokenBudgetForWave(int32 WaveIndex) const
{
	const int32 N = FMath::Max(0, WaveIndex);

	// Base linear growth
	float Tokens = FirstWaveTokens + TokensPerWaveAdd * N;

	// Optional exponential factor
	if (TokensPerWaveMultiplier > 0.f && TokensPerWaveMultiplier != 1.0f)
	{
		const float Growth = FMath::Pow(TokensPerWaveMultiplier, N);
		Tokens *= Growth;
	}

	const int32 Result = FMath::Max(0, FMath::RoundToInt(Tokens));

	UE_LOG(LogTemp, Log,
		TEXT("Waves: ComputeTokenBudgetForWave - Wave=%d Tokens=%d (First=%d Add=%.2f Mult=%.3f)"),
		WaveIndex,
		Result,
		FirstWaveTokens,
		TokensPerWaveAdd,
		TokensPerWaveMultiplier);

	return Result;
}

bool URoundBasedWaveSubsystem::HasAffordableOptions(int32 RemainingTokens) const
{
	if (RemainingTokens <= 0) return false;

	for (const FEnemyTokenOption& Opt : EnemyOptions)
	{
		if (Opt.EnemyClass &&
			Opt.TokenCost > 0 &&
			Opt.TokenCost <= RemainingTokens &&
			Opt.Weight > 0.f)
		{
			return true;
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("Waves: HasAffordableOptions - NONE affordable. RemainingTokens=%d Options=%d"),
		RemainingTokens,
		EnemyOptions.Num());

	return false;
}

bool URoundBasedWaveSubsystem::PickEnemyOption(int32 RemainingTokens, FEnemyTokenOption& OutOption) const
{
	if (RemainingTokens <= 0) return false;

	TArray<const FEnemyTokenOption*> Affordable;
	float TotalWeight = 0.f;

	for (const FEnemyTokenOption& Opt : EnemyOptions)
	{
		if (Opt.EnemyClass &&
			Opt.TokenCost > 0 &&
			Opt.TokenCost <= RemainingTokens &&
			Opt.Weight > 0.f)
		{
			Affordable.Add(&Opt);
			TotalWeight += Opt.Weight;
		}
	}

	if (Affordable.Num() == 0 || TotalWeight <= 0.f)
	{
		UE_LOG(LogTemp, Log,
			TEXT("Waves: PickEnemyOption - No affordable options. RemainingTokens=%d"),
			RemainingTokens);
		return false;
	}

	const float Roll = FMath::FRand() * TotalWeight;
	float Accum = 0.f;

	for (const FEnemyTokenOption* OptPtr : Affordable)
	{
		Accum += OptPtr->Weight;
		if (Roll <= Accum)
		{
			OutOption = *OptPtr;

			UE_LOG(LogTemp, Log,
				TEXT("Waves: PickEnemyOption - Picked Class=%s Cost=%d Weight=%.2f RemainingTokens=%d Roll=%.2f TotalWeight=%.2f"),
				*OutOption.EnemyClass->GetName(),
				OutOption.TokenCost,
				OutOption.Weight,
				RemainingTokens,
				Roll,
				TotalWeight);

			return true;
		}
	}

	OutOption = *Affordable.Last();

	UE_LOG(LogTemp, Log,
		TEXT("Waves: PickEnemyOption - Fallback last Class=%s Cost=%d Weight=%.2f RemainingTokens=%d"),
		*OutOption.EnemyClass->GetName(),
		OutOption.TokenCost,
		OutOption.Weight,
		RemainingTokens);

	return true;
}

AActor* URoundBasedWaveSubsystem::PickSpawner() const
{
	TArray<AActor*> ValidSpawners;
	for (const TWeakObjectPtr<AActor>& Weak : Spawners)
	{
		if (AActor* A = Weak.Get())
		{
			ValidSpawners.Add(A);
		}
	}

	if (ValidSpawners.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Waves: PickSpawner - No valid spawners registered."));
		return nullptr;
	}

	const int32 Index = FMath::RandRange(0, ValidSpawners.Num() - 1);
	AActor* Chosen = ValidSpawners[Index];

	UE_LOG(LogTemp, Log,
		TEXT("Waves: PickSpawner - Chosen %s (Index=%d / %d)"),
		*Chosen->GetName(),
		Index,
		ValidSpawners.Num());

	return Chosen;
}

bool URoundBasedWaveSubsystem::TrySpawnFromTokens()
{
	const int32 TokensRemaining = GetTokensRemaining();

	if (!HasAffordableOptions(TokensRemaining))
	{
		UE_LOG(LogTemp, Log,
			TEXT("Waves: TrySpawnFromTokens - No affordable options. TokensRemaining=%d"),
			TokensRemaining);
		return false;
	}

	FEnemyTokenOption ChosenOption;
	if (!PickEnemyOption(TokensRemaining, ChosenOption))
	{
		UE_LOG(LogTemp, Log,
			TEXT("Waves: TrySpawnFromTokens - PickEnemyOption failed. TokensRemaining=%d"),
			TokensRemaining);
		return false;
	}

	if (!ChosenOption.EnemyClass)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Waves: TrySpawnFromTokens - Chosen option has null EnemyClass. TokensRemaining=%d"),
			TokensRemaining);
		return false;
	}

	AActor* Spawner = PickSpawner();
	if (!Spawner)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Waves: TrySpawnFromTokens - No spawner available to spawn %s."),
			*ChosenOption.EnemyClass->GetName());
		return false;
	}

	UE_LOG(LogTemp, Log,
		TEXT("Waves: TrySpawnFromTokens - Requesting spawn. Wave=%d Spawner=%s EnemyClass=%s Cost=%d TokensRemaining(before spawn)=%d"),
		CurrentWaveIndex,
		*Spawner->GetName(),
		*ChosenOption.EnemyClass->GetName(),
		ChosenOption.TokenCost,
		TokensRemaining);

	// BP_EnemySpawner listens to this delegate, checks if "Self" == Spawner,
	// and then spawns the enemy + calls NotifyEnemySpawned on the subsystem.
	OnRequestSpawn.Broadcast(Spawner, ChosenOption.EnemyClass);

	return true;
}
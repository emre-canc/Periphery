// RoundBasedWaveSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RoundBasedWaveSubsystem.generated.h"

class AActor;
class APawn;

USTRUCT(BlueprintType)
struct FEnemyTokenOption
{
	GENERATED_BODY()

	// Enemy to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wave")
	TSubclassOf<APawn> EnemyClass = nullptr;

	// How many tokens this enemy costs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wave")
	int32 TokenCost = 1;

	// Relative chance when choosing among affordable enemies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wave")
	float Weight = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveFinished, int32, WaveIndex);

// Spawner BP listens to this
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnRequestSpawn,
	AActor*,           Spawner,
	TSubclassOf<APawn>, EnemyClass
);

UCLASS()
class INSIDETFV03_API URoundBasedWaveSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Get from BP: RoundBasedWaveSubsystem::Get(WorldContext)
	UFUNCTION(BlueprintPure, meta=(WorldContext="WorldContextObject"), Category="Waves")
	static URoundBasedWaveSubsystem* Get(const UObject* WorldContextObject);

	//
	// CONFIG – edit on CDO / BP child
	//

	// Tokens for wave 0 (first wave)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Waves|Config")
	int32 FirstWaveTokens = 10;

	// Extra tokens added each wave (linear growth)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Waves|Config")
	float TokensPerWaveAdd = 5.0f;

	// Multiplier applied per wave (exponential growth, 1.0 = off)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Waves|Config")
	float TokensPerWaveMultiplier = 1.0f;

	// 0 = infinite waves
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Waves|Config", meta=(ClampMin="0"))
	int32 MaxWaveCount = 0;

	// Global enemy pool used for all waves
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Waves|Config")
	TArray<FEnemyTokenOption> EnemyOptions;

	UFUNCTION(BlueprintCallable, Category="Waves|Config")
	void SetWaveConfig(
		int32 InFirstWaveTokens,
		float InTokensPerWaveAdd,
		float InTokensPerWaveMultiplier,
		int32 InMaxWaveCount,
		const TArray<FEnemyTokenOption>& InEnemyOptions
	);

	//
	// Spawner registration
	//
	UFUNCTION(BlueprintCallable, Category="Waves|Spawners")
	void RegisterSpawner(AActor* Spawner);

	UFUNCTION(BlueprintCallable, Category="Waves|Spawners")
	void UnregisterSpawner(AActor* Spawner);

	//
	// Wave control
	//
	UFUNCTION(BlueprintCallable, Category="Waves")
	void BeginNextWave();

	UFUNCTION(BlueprintCallable, Category="Waves")
	void ForceEndWave(bool bKillRemaining = false);

	//
	// Enemy lifetime notifications (call from enemies)
	//
	UFUNCTION(BlueprintCallable, Category="Waves|Enemies")
	void NotifyEnemySpawned(APawn* Enemy);

	UFUNCTION(BlueprintCallable, Category="Waves|Enemies")
	void NotifyEnemyDied(APawn* Enemy);

	//
	// Queries
	//
	UFUNCTION(BlueprintPure, Category="Waves")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	UFUNCTION(BlueprintPure, Category="Waves")
	bool IsWaveActive() const { return bWaveActive; }

	UFUNCTION(BlueprintPure, Category="Waves")
	int32 GetTokensRemaining() const { return TokensBudgetThisWave - TokensSpentThisWave; }

	UPROPERTY(BlueprintAssignable, Category="Waves")
	FOnWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category="Waves")
	FOnWaveFinished OnWaveFinished;

	// Subsystem asks spawners to spawn this enemy
	UPROPERTY(BlueprintAssignable, Category="Waves")
	FOnRequestSpawn OnRequestSpawn;

protected:
	virtual void Deinitialize() override;

private:
	// Registered spawners (e.g., BP_EnemySpawner near streetlights)
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> Spawners;

	// Wave state
	UPROPERTY()
	int32 CurrentWaveIndex = -1;

	UPROPERTY()
	int32 TokensBudgetThisWave = 0;

	UPROPERTY()
	int32 TokensSpentThisWave = 0;

	UPROPERTY()
	int32 EnemiesAlive = 0;

	UPROPERTY()
	bool bWaveActive = false;

	// Spawn pacing
	UPROPERTY()
	float SpawnCooldownRemaining = 0.f;

	FTimerHandle SpawnTickHandle;

	// Internal helpers
	void BeginWaveInternal(int32 WaveIndex);
	void EndCurrentWave();
	void SpawnTick();                 // timer → calls TrySpawnFromTokens
	bool TrySpawnFromTokens();        // returns true if a spawn request was issued

	int32 ComputeTokenBudgetForWave(int32 WaveIndex) const;
	bool HasAffordableOptions(int32 RemainingTokens) const;
	bool PickEnemyOption(int32 RemainingTokens, FEnemyTokenOption& OutOption) const;
	AActor* PickSpawner() const;
};
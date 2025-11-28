// Objective_Simple.h
#pragma once
#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "Objective_Simple.generated.h"

UCLASS(DisplayName = "Simple (Wait for Event)")
class INSIDETFV03_API UObjective_Simple : public UMissionObjective
{
    GENERATED_BODY()
public:
    // ... UPROPERTIES ...
    virtual bool OnEvent(...) const override;
    virtual bool IsComplete(...) const override;
};
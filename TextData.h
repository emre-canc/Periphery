// Periphery -- EvEGames -- TextData.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TextData.generated.h"

USTRUCT(BlueprintType)
struct FTextLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true))
    FText Text;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FLinearColor TextColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<USoundBase> VoiceLine; // Audio to play
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float TextDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bTypewriter = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float TypewriterSpeed = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<USoundBase> TextSound; // Sound when the text appears. For typewriter mode it will trigger for each letter.
};

UCLASS(BlueprintType)
class INSIDETFV03_API UTextData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    TArray<FTextLine> Lines;
};
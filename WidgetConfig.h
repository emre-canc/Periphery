#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WidgetConfig.generated.h"

// Abstract base class. You create children of this for each complex widget type.
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class INSIDETFV03_API UWidgetConfig : public UObject
{
    GENERATED_BODY()

public:
    // Every payload must tell us what Widget Class it spawns

    UPROPERTY(EditDefaultsOnly, Category = "Config")
    TSubclassOf<UUserWidget> WidgetClass;

    virtual TSubclassOf<UUserWidget> GetWidgetClass() const { return WidgetClass; }


    // Every payload knows how to take that spawned widget and setup variables
    UFUNCTION(BlueprintNativeEvent, Category = "UI")
    void InitializeWidget(UUserWidget* NewWidget) const;

    virtual void InitializeWidget_Implementation(UUserWidget* NewWidget) const
    {
        // Default C++ logic (optional)
    }
};
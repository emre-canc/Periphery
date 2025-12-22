#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Widget/WidgetStructs.h"
#include "WidgetInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UWidgetInterface : public UInterface
{
    GENERATED_BODY()
};

class INSIDETFV03_API IWidgetInterface
{
    GENERATED_BODY()

public:
    // --- API Functions ---

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WidgetInterface|Commands")
    void OnConfirm();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WidgetInterface|Commands")
    void OnCancel(bool& bSuccess);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WidgetInterface|Rendering")
    void SetWidgetVisibility(bool bVisible);


};
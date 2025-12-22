#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h"
#include "PeripheryGameMode.generated.h"

/**
 * 
 */
UCLASS()
class INSIDETFV03_API APeripheryGameMode : public AGameModeBase
{
	GENERATED_BODY()
	

public:

    UPROPERTY(EditDefaultsOnly, Category = "Setup")
    FGameplayTag MenuCameraTag;

    UPROPERTY(EditDefaultsOnly, Category = "Setup")
    FGameplayTag PlayerCameraTag;

    UPROPERTY(EditDefaultsOnly, Category = "Setup")
    FGameplayTag PrologueMissionTag;

    UPROPERTY(EditDefaultsOnly, Category = "Setup|Menu")
    TSubclassOf<UUserWidget> MainMenuClass;

    UPROPERTY(EditDefaultsOnly, Category = "Setup|Menu")
    bool bShowMenu;


    // The function the UI calls to start the game
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartSessionFromMenu();

    UFUNCTION(BlueprintPure, Category = "Game Flow")
    UUserWidget* GetCurrentMenuWidget() const { return CurrentMenuWidget; }



protected:
    virtual void BeginPlay() override;

    UUserWidget* CurrentMenuWidget;


};
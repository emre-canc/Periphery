
#include "Items/BaseLadder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"


// Sets default values
ABaseLadder::ABaseLadder()
{
     // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    LadderRoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Root"));
    RootComponent = LadderRoot;

    LadderTop = CreateDefaultSubobject<USceneComponent>(TEXT("Top"));
    LadderTop->SetupAttachment(RootComponent);
    LadderBottom = CreateDefaultSubobject<USceneComponent>(TEXT("Bottom"));
    LadderBottom->SetupAttachment(RootComponent);
    
}



FVector ABaseLadder::GetLadderTop() const { return LadderTop->GetComponentLocation(); }
FVector ABaseLadder::GetLadderBottom() const { return LadderBottom->GetComponentLocation(); }



// Called when the game starts or when spawned
void ABaseLadder::BeginPlay()
{
    Super::BeginPlay();
    
}



// Called every frame
void ABaseLadder::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}


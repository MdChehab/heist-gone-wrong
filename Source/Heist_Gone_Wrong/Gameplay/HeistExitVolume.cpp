// Heist Gone Wrong

#include "HeistExitVolume.h"
#include "HeistObjectiveSubsystem.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AHeistExitVolume::AHeistExitVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Trigger->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	// Overlap pawns, block nothing.
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
}

void AHeistExitVolume::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		Objective = World->GetSubsystem<UHeistObjectiveSubsystem>();
	}

	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AHeistExitVolume::OnTriggerBeginOverlap);
}

void AHeistExitVolume::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn == nullptr || !Pawn->IsPlayerControlled() || Objective == nullptr)
	{
		return;
	}

	if (Objective->HasArtifact())
	{
		Objective->NotifyReachedExit();
	}
#if !UE_BUILD_SHIPPING
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Steal the artifact before escaping"));
	}
#endif
}

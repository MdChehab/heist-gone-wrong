// Heist Gone Wrong

#include "HeistCheckpoint.h"
#include "HeistObjectiveSubsystem.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

AHeistCheckpoint::AHeistCheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Trigger->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
}

void AHeistCheckpoint::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		Objective = World->GetSubsystem<UHeistObjectiveSubsystem>();
	}

	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AHeistCheckpoint::OnTriggerBeginOverlap);
}

void AHeistCheckpoint::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bReached || Objective == nullptr)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn == nullptr || !Pawn->IsPlayerControlled())
	{
		return;
	}

	bReached = true;
	Objective->SetCheckpoint(GetActorTransform());
}

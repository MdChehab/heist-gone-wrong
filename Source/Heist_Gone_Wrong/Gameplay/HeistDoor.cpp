// Heist Gone Wrong

#include "HeistDoor.h"
#include "HeistObjectiveSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AHeistDoor::AHeistDoor()
{
	// Ticks only during an open/close transition; disabled when settled.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	DoorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
	SetRootComponent(DoorRoot);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(DoorRoot);
}

void AHeistDoor::BeginPlay()
{
	Super::BeginPlay();

	// The designer places the closed panel; capture that as the closed pose.
	ClosedRelLocation = DoorMesh->GetRelativeLocation();
	ClosedRelRotation = DoorMesh->GetRelativeRotation();

	bTargetOpen = bStartsOpen;
	bSavedOpen = bStartsOpen;
	OpenAlpha = bStartsOpen ? 1.f : 0.f;
	ApplyAlpha(OpenAlpha);

	if (UWorld* World = GetWorld())
	{
		if (UHeistObjectiveSubsystem* Objective = World->GetSubsystem<UHeistObjectiveSubsystem>())
		{
			Objective->OnRunReset.AddDynamic(this, &AHeistDoor::HandleRunReset);
			Objective->OnCheckpointSaved.AddDynamic(this, &AHeistDoor::HandleCheckpointSaved);
		}
	}
}

void AHeistDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float Target = bTargetOpen ? 1.f : 0.f;
	const float Step = DeltaSeconds / OpenDuration;
	OpenAlpha = FMath::FInterpConstantTo(OpenAlpha, Target, 1.f, Step);

	ApplyAlpha(OpenAlpha);

	if (FMath::IsNearlyEqual(OpenAlpha, Target))
	{
		OpenAlpha = Target;
		SetActorTickEnabled(false);
	}
}

void AHeistDoor::ApplyAlpha(float Alpha)
{
	DoorMesh->SetRelativeLocation(ClosedRelLocation + OpenLocationOffset * Alpha);
	DoorMesh->SetRelativeRotation(ClosedRelRotation + OpenRotationOffset * Alpha);
}

void AHeistDoor::SetTargetOpen(bool bOpen)
{
	if (bTargetOpen == bOpen && FMath::IsNearlyEqual(OpenAlpha, bOpen ? 1.f : 0.f))
	{
		return;
	}

	bTargetOpen = bOpen;
	SetActorTickEnabled(true);

	USoundBase* Sound = bOpen ? OpenSound : CloseSound;
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
	}
}

void AHeistDoor::OpenDoor()
{
	SetTargetOpen(true);
}

void AHeistDoor::CloseDoor()
{
	SetTargetOpen(false);
}

void AHeistDoor::Activate_Implementation(AActor* Activator)
{
	OpenDoor();
}

void AHeistDoor::Deactivate_Implementation(AActor* Activator)
{
	CloseDoor();
}

void AHeistDoor::HandleRunReset()
{
	// Snap back to the banked (checkpoint) state instantly on a restart.
	bTargetOpen = bSavedOpen;
	OpenAlpha = bSavedOpen ? 1.f : 0.f;
	ApplyAlpha(OpenAlpha);
	SetActorTickEnabled(false);
}

void AHeistDoor::HandleCheckpointSaved()
{
	bSavedOpen = bTargetOpen;
}

// Heist Gone Wrong

#include "HeistArtifact.h"
#include "HeistObjectiveSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AHeistArtifact::AHeistArtifact()
{
	PrimaryActorTick.bCanEverTick = false;

	ArtifactMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArtifactMesh"));
	SetRootComponent(ArtifactMesh);

	InteractionPrompt = NSLOCTEXT("Heist", "ArtifactPrompt", "Steal Artifact");
}

void AHeistArtifact::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		Objective = World->GetSubsystem<UHeistObjectiveSubsystem>();
		if (Objective)
		{
			Objective->OnRunReset.AddDynamic(this, &AHeistArtifact::HandleRunReset);
			Objective->OnCheckpointSaved.AddDynamic(this, &AHeistArtifact::HandleCheckpointSaved);
		}
	}
}

FText AHeistArtifact::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}

bool AHeistArtifact::CanInteract_Implementation(AActor* Interactor) const
{
	return !bTaken;
}

void AHeistArtifact::Interact_Implementation(AActor* Interactor)
{
	if (bTaken)
	{
		return;
	}

	if (Objective)
	{
		Objective->PickUpArtifact();
	}

	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}

	SetTaken(true);
}

void AHeistArtifact::SetTaken(bool bInTaken)
{
	bTaken = bInTaken;
	SetActorHiddenInGame(bTaken);
	SetActorEnableCollision(!bTaken);
}

void AHeistArtifact::HandleRunReset()
{
	// Restore to the banked (checkpoint) state: back in the world unless it had
	// already been taken by the time the player reached the checkpoint.
	SetTaken(bSavedTaken);
}

void AHeistArtifact::HandleCheckpointSaved()
{
	bSavedTaken = bTaken;
}

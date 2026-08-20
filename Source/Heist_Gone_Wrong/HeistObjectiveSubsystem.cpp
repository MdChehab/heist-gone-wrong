// Heist Gone Wrong

#include "HeistObjectiveSubsystem.h"
#include "AI/HeistDetectionSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

void UHeistObjectiveSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// Restart the run whenever the detection meter fills.
	DetectionSubsystem = World->GetSubsystem<UHeistDetectionSubsystem>();
	if (DetectionSubsystem)
	{
		DetectionSubsystem->OnPlayerDetected.AddDynamic(this, &UHeistObjectiveSubsystem::HandlePlayerDetected);
	}

	// Default the checkpoint to the PlayerStart; checkpoint actors can override it.
	if (!bCheckpointSet)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			CheckpointTransform = It->GetActorTransform();
			bCheckpointSet = true;
			break;
		}
	}
}

void UHeistObjectiveSubsystem::Deinitialize()
{
	if (DetectionSubsystem)
	{
		DetectionSubsystem->OnPlayerDetected.RemoveDynamic(this, &UHeistObjectiveSubsystem::HandlePlayerDetected);
	}

	Super::Deinitialize();
}

void UHeistObjectiveSubsystem::SetCheckpoint(const FTransform& NewCheckpoint)
{
	CheckpointTransform = NewCheckpoint;
	bCheckpointSet = true;

	// Bank the current progress: artifact state here, and every gameplay actor
	// snapshots itself so a later failure restores to this point, not the start.
	bSavedHasArtifact = bHasArtifact;
	OnCheckpointSaved.Broadcast();
}

void UHeistObjectiveSubsystem::ShowHint(const FText& Message, float Duration)
{
	CurrentHint = Message;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HintTimerHandle, this, &UHeistObjectiveSubsystem::ClearHint, FMath::Max(Duration, 0.1f), false);
	}
}

void UHeistObjectiveSubsystem::ClearHint()
{
	CurrentHint = FText::GetEmpty();
}

FText UHeistObjectiveSubsystem::GetObjectiveText() const
{
	return bHasArtifact
		? NSLOCTEXT("Heist", "ObjectiveEscape", "Escape to the exit")
		: NSLOCTEXT("Heist", "ObjectiveSteal", "Steal the artifact");
}

void UHeistObjectiveSubsystem::PickUpArtifact()
{
	if (RunState != EHeistRunState::Playing || bHasArtifact)
	{
		return;
	}

	bHasArtifact = true;
	OnObjectiveChanged.Broadcast(NSLOCTEXT("Heist", "ObjectiveEscape", "Escape to the exit"));
}

void UHeistObjectiveSubsystem::NotifyReachedExit()
{
	if (RunState == EHeistRunState::Playing && bHasArtifact)
	{
		CompleteRun();
	}
}

void UHeistObjectiveSubsystem::CompleteRun()
{
	RunState = EHeistRunState::Won;
	OnRunStateChanged.Broadcast(RunState);
}

void UHeistObjectiveSubsystem::HandlePlayerDetected()
{
	RestartFromCheckpoint();
}

void UHeistObjectiveSubsystem::RestartFromCheckpoint()
{
	if (RunState != EHeistRunState::Playing)
	{
		return;
	}

	// Restore progress to the last checkpoint (start, if none reached yet).
	bHasArtifact = bSavedHasArtifact;

	RespawnPlayerAtCheckpoint();

	if (DetectionSubsystem)
	{
		DetectionSubsystem->ResetDetection();
	}

	// Guards, the door and the artifact restore to their checkpoint snapshots.
	OnRunReset.Broadcast();
	OnObjectiveChanged.Broadcast(bHasArtifact
		? NSLOCTEXT("Heist", "ObjectiveEscape", "Escape to the exit")
		: NSLOCTEXT("Heist", "ObjectiveSteal", "Steal the artifact"));
}

void UHeistObjectiveSubsystem::RespawnPlayerAtCheckpoint()
{
	UWorld* World = GetWorld();
	if (World == nullptr || !bCheckpointSet)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
	if (!IsValid(PlayerPawn))
	{
		return;
	}

	PlayerPawn->SetActorTransform(CheckpointTransform, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);

	if (ACharacter* PlayerChar = Cast<ACharacter>(PlayerPawn))
	{
		PlayerChar->GetCharacterMovement()->StopMovementImmediately();
	}
}

// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HeistObjectiveSubsystem.generated.h"

class UHeistDetectionSubsystem;

/** Overall state of the current run. */
UENUM(BlueprintType)
enum class EHeistRunState : uint8
{
	Playing UMETA(DisplayName="Playing"),
	Won     UMETA(DisplayName="Won")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHeistRunStateChanged, EHeistRunState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHeistObjectiveChanged, const FText&, ObjectiveText);

/** Broadcast when the run restarts from a checkpoint, so gameplay actors restore themselves. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHeistRunReset);

/** Broadcast when a checkpoint is reached, so gameplay actors snapshot their current state. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHeistCheckpointSaved);

/**
 *  The run's brain: tracks the objective (steal the artifact, then escape),
 *  owns the checkpoint, and drives fail/restart and win. It listens to the
 *  detection subsystem and restarts the run when the player is fully detected.
 *
 *  A world subsystem, like the detection meter: this is game-wide run state that
 *  many actors read and write, so it does not belong on any single actor.
 */
UCLASS()
class UHeistObjectiveSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	//~ Begin USubsystem / UWorldSubsystem
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	//~ End

	/** The artifact was picked up. Objective becomes "reach the exit". */
	UFUNCTION(BlueprintCallable, Category="Objective")
	void PickUpArtifact();

	UFUNCTION(BlueprintCallable, Category="Objective")
	bool HasArtifact() const { return bHasArtifact; }

	/** The player entered the exit. Wins the run if the artifact has been taken. */
	UFUNCTION(BlueprintCallable, Category="Objective")
	void NotifyReachedExit();

	/** Restart the run from the checkpoint (also invoked automatically on detection). */
	UFUNCTION(BlueprintCallable, Category="Objective")
	void RestartFromCheckpoint();

	/** Update where the player respawns (checkpoint actors call this). */
	UFUNCTION(BlueprintCallable, Category="Objective")
	void SetCheckpoint(const FTransform& NewCheckpoint);

	UFUNCTION(BlueprintCallable, Category="Objective")
	EHeistRunState GetRunState() const { return RunState; }

	/** Bind the HUD run-state handler here (win screen in W6) */
	UPROPERTY(BlueprintAssignable, Category="Objective")
	FHeistRunStateChanged OnRunStateChanged;

	/** Bind the HUD objective text here */
	UPROPERTY(BlueprintAssignable, Category="Objective")
	FHeistObjectiveChanged OnObjectiveChanged;

	/** Guards, the door and the artifact bind here to restore themselves on restart */
	UPROPERTY(BlueprintAssignable, Category="Objective")
	FHeistRunReset OnRunReset;

	/** Gameplay actors bind here to snapshot their state when a checkpoint is reached */
	UPROPERTY(BlueprintAssignable, Category="Objective")
	FHeistCheckpointSaved OnCheckpointSaved;

private:

	/** Bound to the detection subsystem: full detection restarts the run */
	UFUNCTION()
	void HandlePlayerDetected();

	void RespawnPlayerAtCheckpoint();
	void CompleteRun();

	UPROPERTY(Transient)
	TObjectPtr<UHeistDetectionSubsystem> DetectionSubsystem;

	EHeistRunState RunState = EHeistRunState::Playing;
	bool bHasArtifact = false;

	/** Artifact state banked at the last checkpoint, restored on a restart */
	bool bSavedHasArtifact = false;

	FTransform CheckpointTransform;
	bool bCheckpointSet = false;
};

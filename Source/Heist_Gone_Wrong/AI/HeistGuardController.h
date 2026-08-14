// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HeistGuardController.generated.h"

class AHeistGuardCharacter;

/**
 *  Guard behaviour states. Only Patrol is implemented in W3; Investigate and
 *  Alerted are reserved for W4 (react to a thrown-object noise, and full
 *  detection) and are declared here so the state plumbing exists up front.
 */
UENUM(BlueprintType)
enum class EGuardState : uint8
{
	Patrol      UMETA(DisplayName="Patrol"),
	Investigate UMETA(DisplayName="Investigate"),
	Alerted     UMETA(DisplayName="Alerted")
};

/**
 *  The guard's brain. Drives a simple C++ state machine and issues navmesh moves.
 *  Movement completion is handled through OnMoveCompleted rather than Tick, so
 *  the guard only does work when it actually reaches a waypoint.
 */
UCLASS()
class AHeistGuardController : public AAIController
{
	GENERATED_BODY()

public:

	AHeistGuardController();

	UFUNCTION(BlueprintCallable, Category="Guard")
	EGuardState GetGuardState() const { return GuardState; }

protected:

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	/** Enter Patrol and head for the first waypoint */
	void StartPatrol();

	/** Issue a MoveTo for the current patrol index */
	void MoveToCurrentPatrolPoint();

	/** Step the patrol index (loop or ping-pong) and move to the next point */
	void AdvancePatrol();

private:

	/** Possessed guard, resolved in OnPossess */
	UPROPERTY(Transient)
	TObjectPtr<AHeistGuardCharacter> GuardPawn;

	EGuardState GuardState = EGuardState::Patrol;

	/** Index into the guard's PatrolPoints */
	int32 PatrolIndex = 0;

	/** +1 or -1, used only when the route reverses instead of looping */
	int32 PatrolDirection = 1;

	FTimerHandle WaitTimerHandle;
};

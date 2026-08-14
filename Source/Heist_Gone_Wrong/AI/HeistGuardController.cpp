// Heist Gone Wrong

#include "HeistGuardController.h"
#include "HeistGuardCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Heist_Gone_Wrong.h"

AHeistGuardController::AHeistGuardController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHeistGuardController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GuardPawn = Cast<AHeistGuardCharacter>(InPawn);
	if (!IsValid(GuardPawn))
	{
		UE_LOG(LogHeist_Gone_Wrong, Warning,
			TEXT("%s possessed a non-guard pawn '%s'; no patrol will run."),
			*GetName(), *GetNameSafe(InPawn));
		return;
	}

	StartPatrol();
}

void AHeistGuardController::OnUnPossess()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WaitTimerHandle);
	}

	GuardPawn = nullptr;
	Super::OnUnPossess();
}

void AHeistGuardController::StartPatrol()
{
	GuardState = EGuardState::Patrol;
	PatrolIndex = 0;
	PatrolDirection = 1;

	if (!IsValid(GuardPawn) || GuardPawn->GetPatrolPoints().Num() == 0)
	{
		UE_LOG(LogHeist_Gone_Wrong, Warning,
			TEXT("%s has no patrol points assigned; guard will stand idle."),
			*GetNameSafe(GuardPawn));
		return;
	}

	MoveToCurrentPatrolPoint();
}

void AHeistGuardController::MoveToCurrentPatrolPoint()
{
	if (!IsValid(GuardPawn))
	{
		return;
	}

	const TArray<TObjectPtr<AActor>>& Points = GuardPawn->GetPatrolPoints();
	if (!Points.IsValidIndex(PatrolIndex))
	{
		return;
	}

	AActor* Target = Points[PatrolIndex];
	if (!IsValid(Target))
	{
		// A null slot in the array should not stall the whole patrol.
		UE_LOG(LogHeist_Gone_Wrong, Warning,
			TEXT("%s patrol point %d is not set; skipping it."), *GetNameSafe(GuardPawn), PatrolIndex);
		AdvancePatrol();
		return;
	}

	MoveToActor(Target, GuardPawn->GetPatrolAcceptanceRadius());
}

void AHeistGuardController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	// W4 will branch here on Investigate/Alerted; for now only patrol reacts.
	if (GuardState != EGuardState::Patrol || !IsValid(GuardPawn))
	{
		return;
	}

	// Always defer the next step through a timer, even when the wait is zero.
	// A synchronous MoveTo failure would otherwise recurse straight back into
	// OnMoveCompleted; a timer breaks that into separate frames. The wait also
	// gives the guard its readable pause-and-look-around beat at each point.
	if (UWorld* World = GetWorld())
	{
		const float WaitTime = FMath::Max(GuardPawn->GetWaitTimeAtPoint(), 0.f);
		World->GetTimerManager().SetTimer(
			WaitTimerHandle, this, &AHeistGuardController::AdvancePatrol, WaitTime + KINDA_SMALL_NUMBER, false);
	}
}

void AHeistGuardController::AdvancePatrol()
{
	if (!IsValid(GuardPawn))
	{
		return;
	}

	const int32 Num = GuardPawn->GetPatrolPoints().Num();
	if (Num == 0)
	{
		return;
	}

	if (Num > 1)
	{
		if (GuardPawn->ShouldLoopPatrol())
		{
			PatrolIndex = (PatrolIndex + 1) % Num;
		}
		else
		{
			// Reverse at either end so the guard walks the route back and forth.
			if (PatrolIndex + PatrolDirection < 0 || PatrolIndex + PatrolDirection >= Num)
			{
				PatrolDirection = -PatrolDirection;
			}
			PatrolIndex += PatrolDirection;
		}
	}

	MoveToCurrentPatrolPoint();
}

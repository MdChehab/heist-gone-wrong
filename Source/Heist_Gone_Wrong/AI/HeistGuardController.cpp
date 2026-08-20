// Heist Gone Wrong

#include "HeistGuardController.h"
#include "HeistGuardCharacter.h"
#include "HeistDetectionSubsystem.h"
#include "HeistObjectiveSubsystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Heist_Gone_Wrong.h"

AHeistGuardController::AHeistGuardController()
{
	// The controller must keep its default tick: AAIController updates its
	// control rotation from the pawn's orientation there, and AI Perception
	// takes the sight-cone direction from that control rotation. Disabling it
	// freezes the vision cone to the guard's spawn facing. Our own gameplay is
	// still event/timer driven; this is engine machinery, not per-frame logic.

	GuardPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("GuardPerception"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	ConfigureSenses();

	GuardPerception->ConfigureSense(*SightConfig);
	GuardPerception->ConfigureSense(*HearingConfig);
	GuardPerception->SetDominantSense(UAISenseConfig_Sight::StaticClass());
	SetPerceptionComponent(*GuardPerception);
}

void AHeistGuardController::ConfigureSenses()
{
	// Sight cone + line of sight (AI Perception traces for us).
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = FMath::Max(LoseSightRadius, SightRadius);
	SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionHalfAngle;
	SightConfig->SetMaxAge(5.f);
	// The player is not on a team, so detect neutrals too or the guard sees nothing.
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	HearingConfig->HearingRange = HearingRange;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
}

void AHeistGuardController::BeginPlay()
{
	Super::BeginPlay();

	// Re-apply tuning (a BP subclass may override the defaults) and bind events.
	ConfigureSenses();
	GuardPerception->ConfigureSense(*SightConfig);
	GuardPerception->ConfigureSense(*HearingConfig);
	GuardPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AHeistGuardController::HandlePerceptionUpdated);

	if (UWorld* World = GetWorld())
	{
		DetectionSubsystem = World->GetSubsystem<UHeistDetectionSubsystem>();

		// Return to patrol whenever the run restarts from a checkpoint.
		if (UHeistObjectiveSubsystem* Objective = World->GetSubsystem<UHeistObjectiveSubsystem>())
		{
			Objective->OnRunReset.AddDynamic(this, &AHeistGuardController::ResetToPatrol);
		}
	}
}

void AHeistGuardController::ResetToPatrol()
{
	StopMovement();

	SeenPlayer = nullptr;
	bInvestigatingPlayer = false;
	PatrolIndex = 0;
	PatrolDirection = 1;

	if (IsValid(GuardPawn))
	{
		GuardPawn->ResetToStart();
	}

	EnterPatrol();
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

	EnterPatrol();
}

void AHeistGuardController::OnUnPossess()
{
	ClearWaitTimer();
	ReportSeesPlayer(false);
	GuardPawn = nullptr;
	Super::OnUnPossess();
}

// ---------------------------------------------------------------------------
// State entry
// ---------------------------------------------------------------------------

void AHeistGuardController::EnterPatrol()
{
	GuardState = EGuardState::Patrol;
	ClearWaitTimer();
	ClearFocus(EAIFocusPriority::Gameplay);
	ReportSeesPlayer(false);
	SetVisionWide(false);
	if (IsValid(GuardPawn))
	{
		GuardPawn->ApplyPatrolSpeed();
		GuardPawn->SetFaceControlRotation(false);
	}

	if (!IsValid(GuardPawn) || GuardPawn->GetPatrolPoints().Num() == 0)
	{
		UE_LOG(LogHeist_Gone_Wrong, Warning,
			TEXT("%s has no patrol points assigned; guard will stand idle."),
			*GetNameSafe(GuardPawn));
		return;
	}

	MoveToCurrentPatrolPoint();
}

void AHeistGuardController::EnterInvestigate(const FVector& PointOfInterest, bool bFromNoise)
{
	// Bark a suspicious line only when first becoming suspicious, not when a new
	// noise re-targets a guard that is already investigating.
	const bool bWasInvestigating = (GuardState == EGuardState::Investigate);

	GuardState = EGuardState::Investigate;
	bInvestigatingPlayer = !bFromNoise;
	ClearWaitTimer();
	ClearFocus(EAIFocusPriority::Gameplay);
	ReportSeesPlayer(false);
	SetVisionWide(false);
	if (IsValid(GuardPawn))
	{
		GuardPawn->ApplyInvestigateSpeed();
		GuardPawn->SetFaceControlRotation(false);
		if (!bWasInvestigating)
		{
			GuardPawn->PlaySuspicionBark();
		}
	}

	// Push the destination a little past the point of interest, along the
	// guard's approach, so it walks through a doorway into the room rather than
	// stopping on the threshold where a wall clips its cone. Only for a player
	// hunt - a guard chasing a noise should go to the sound, not past it.
	// Projecting to the navmesh keeps the overshot point reachable.
	FVector Target = PointOfInterest;
	if (!bFromNoise && IsValid(GuardPawn))
	{
		FVector Approach = PointOfInterest - GuardPawn->GetActorLocation();
		Approach.Z = 0.f;
		const FVector ApproachDir = Approach.GetSafeNormal();
		if (!ApproachDir.IsNearlyZero())
		{
			Target = PointOfInterest + ApproachDir * InvestigateOvershoot;
		}
	}

	DrawGuardDebug(Target, bFromNoise);

	MoveToLocation(Target, InvestigateAcceptanceRadius,
		/*bStopOnOverlap*/ true, /*bUsePathfinding*/ true, /*bProjectDestinationToNavigation*/ true);
}

void AHeistGuardController::EnterAlerted()
{
	// Bark an alert line only on the transition into Alerted, not on every
	// perception refresh while the guard already sees the player.
	const bool bWasAlerted = (GuardState == EGuardState::Alerted);

	GuardState = EGuardState::Alerted;
	ClearWaitTimer();
	SetVisionWide(false);

	if (!bWasAlerted && IsValid(GuardPawn))
	{
		GuardPawn->PlayAlertBark();
	}

	// Advance on the player rather than standing still: no combat, but the guard
	// closes in and faces them while the detection meter fills. MoveToActor
	// tracks the moving player; losing sight drops the guard to Investigate.
	if (IsValid(GuardPawn))
	{
		GuardPawn->ApplyChaseSpeed();
		GuardPawn->SetFaceControlRotation(true);
	}
	if (SeenPlayer.IsValid())
	{
		SetFocus(SeenPlayer.Get(), EAIFocusPriority::Gameplay);
		MoveToActor(SeenPlayer.Get(), ChaseAcceptanceRadius);
	}
	ReportSeesPlayer(true);

#if !UE_BUILD_SHIPPING
	if (bDebugGuard && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(GetUniqueID()), 1.5f, FColor::Red, FString::Printf(TEXT("%s: ALERTED (sees player)"), *GetName()));
	}
#endif
}

// ---------------------------------------------------------------------------
// Patrol
// ---------------------------------------------------------------------------

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
		UE_LOG(LogHeist_Gone_Wrong, Warning,
			TEXT("%s patrol point %d is not set; skipping it."), *GetNameSafe(GuardPawn), PatrolIndex);
		AdvancePatrol();
		return;
	}

	MoveToActor(Target, GuardPawn->GetPatrolAcceptanceRadius());
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
			if (PatrolIndex + PatrolDirection < 0 || PatrolIndex + PatrolDirection >= Num)
			{
				PatrolDirection = -PatrolDirection;
			}
			PatrolIndex += PatrolDirection;
		}
	}

	MoveToCurrentPatrolPoint();
}

void AHeistGuardController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	UWorld* World = GetWorld();
	if (World == nullptr || !IsValid(GuardPawn))
	{
		return;
	}

	// Alerted holds position, so it issues no moves to complete here.
	if (GuardState == EGuardState::Patrol)
	{
		// Deferred through a timer even at zero wait, so a synchronous move
		// failure cannot recurse straight back into this handler.
		const float WaitTime = FMath::Max(GuardPawn->GetWaitTimeAtPoint(), 0.f);
		World->GetTimerManager().SetTimer(
			WaitTimerHandle, this, &AHeistGuardController::AdvancePatrol, WaitTime + KINDA_SMALL_NUMBER, false);
	}
	else if (GuardState == EGuardState::Investigate)
	{
		// Reached the point of interest: survey the area, then resume patrol.
		StartLookAround();
	}
}

void AHeistGuardController::StartLookAround()
{
	if (!IsValid(GuardPawn))
	{
		EnterPatrol();
		return;
	}

	// Widen the cone so the guard takes in the whole area without turning (no
	// slide), then resume patrol after the look-around beat. The look-around
	// animation plays over this window. A heard noise holds the guard longer
	// than a player hunt, so the distraction actually buys time.
	SetVisionWide(true);
	GuardPawn->PlayInvestigateMontage();

	const float BaseTime = bInvestigatingPlayer ? LookAroundTime : NoiseInvestigateTime;
	const float LookTime = FMath::Max(BaseTime, KINDA_SMALL_NUMBER);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			WaitTimerHandle, this, &AHeistGuardController::EnterPatrol, LookTime, false);
	}
}

void AHeistGuardController::SetVisionWide(bool bWide)
{
	if (SightConfig == nullptr || GuardPerception == nullptr)
	{
		return;
	}

	const float DesiredAngle = bWide ? InvestigateVisionHalfAngle : PeripheralVisionHalfAngle;
	if (FMath::IsNearlyEqual(SightConfig->PeripheralVisionAngleDegrees, DesiredAngle))
	{
		return;
	}

	SightConfig->PeripheralVisionAngleDegrees = DesiredAngle;
	GuardPerception->ConfigureSense(*SightConfig);
	GuardPerception->RequestStimuliListenerUpdate();
}

// ---------------------------------------------------------------------------
// Perception
// ---------------------------------------------------------------------------

void AHeistGuardController::HandlePerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		APawn* SightedPawn = Cast<APawn>(Actor);
		// Only the player matters; ignore other guards or props.
		if (SightedPawn == nullptr || !SightedPawn->IsPlayerControlled())
		{
			return;
		}

		if (Stimulus.WasSuccessfullySensed())
		{
			OnSawPlayer(SightedPawn);
		}
		else
		{
			OnLostPlayer();
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			OnHeardNoise(Stimulus.StimulusLocation);
		}
	}
}

void AHeistGuardController::OnSawPlayer(APawn* Player)
{
	SeenPlayer = Player;
	EnterAlerted();
}

void AHeistGuardController::OnLostPlayer()
{
	// Investigate where the player was last seen, then fall back to patrol.
	const FVector LastKnown = SeenPlayer.IsValid()
		? SeenPlayer->GetActorLocation()
		: (IsValid(GuardPawn) ? GuardPawn->GetActorLocation() : FVector::ZeroVector);

	SeenPlayer = nullptr;

	if (GuardState == EGuardState::Alerted)
	{
		EnterInvestigate(LastKnown, /*bFromNoise*/ false);
	}
}

void AHeistGuardController::OnHeardNoise(const FVector& NoiseLocation)
{
	// A guard watching the player, or already hunting where it last saw the
	// player, does not get pulled away by a noise. A noise can still interrupt
	// patrol or an earlier noise investigation.
	if (GuardState == EGuardState::Alerted)
	{
		return;
	}
	if (GuardState == EGuardState::Investigate && bInvestigatingPlayer)
	{
		return;
	}

	EnterInvestigate(NoiseLocation, /*bFromNoise*/ true);
}

void AHeistGuardController::DrawGuardDebug(const FVector& InvestigatePoint, bool bFromNoise) const
{
#if !UE_BUILD_SHIPPING
	if (!bDebugGuard)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// Cyan = heading to a heard noise (the intended distraction). Orange = heading
	// to where the player was last seen. This tells us which sense drove the guard.
	const FColor Color = bFromNoise ? FColor::Cyan : FColor::Orange;
	DrawDebugSphere(World, InvestigatePoint, 50.f, 12, Color, false, 6.f, 0, 3.f);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(GetUniqueID()), 3.f, Color,
			FString::Printf(TEXT("%s: INVESTIGATE (%s)"), *GetName(),
				bFromNoise ? TEXT("heard noise") : TEXT("lost sight of player")));
	}
#endif
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void AHeistGuardController::ReportSeesPlayer(bool bSeesPlayer)
{
	if (DetectionSubsystem && IsValid(GuardPawn))
	{
		DetectionSubsystem->SetGuardSeesPlayer(GuardPawn, bSeesPlayer);
	}
}

void AHeistGuardController::ClearWaitTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WaitTimerHandle);
	}
}

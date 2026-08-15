// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HeistGuardController.generated.h"

class AHeistGuardCharacter;
class UHeistDetectionSubsystem;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
struct FAIStimulus;

/**
 *  Guard behaviour states.
 *  - Patrol: walk the waypoint route.
 *  - Investigate: go to a point of interest (a heard noise or the player's last
 *    known position), look around, then resume patrol.
 *  - Alerted: the player is currently in sight. Stop and face them while the
 *    global detection meter fills.
 */
UENUM(BlueprintType)
enum class EGuardState : uint8
{
	Patrol      UMETA(DisplayName="Patrol"),
	Investigate UMETA(DisplayName="Investigate"),
	Alerted     UMETA(DisplayName="Alerted")
};

/**
 *  The guard's brain. Drives a C++ state machine, senses the player through AI
 *  Perception (sight + hearing), and feeds the detection subsystem while the
 *  player is in view. Everything is event / timer driven; no Tick.
 */
UCLASS()
class AHeistGuardController : public AAIController
{
	GENERATED_BODY()

public:

	AHeistGuardController();

	UFUNCTION(BlueprintCallable, Category="Guard")
	EGuardState GetGuardState() const { return GuardState; }

	/** Forget the player and return to the patrol start (checkpoint restart) */
	UFUNCTION()
	void ResetToPatrol();

protected:

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void BeginPlay() override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	// --- Vision / hearing tuning (applied to the sense configs in BeginPlay) ---

	/** How far the guard can see, in cm */
	UPROPERTY(EditDefaultsOnly, Category="Guard|Vision", meta=(ClampMin="0"))
	float SightRadius = 1400.f;

	/** Distance at which an already-seen target is lost, in cm (>= SightRadius) */
	UPROPERTY(EditDefaultsOnly, Category="Guard|Vision", meta=(ClampMin="0"))
	float LoseSightRadius = 1700.f;

	/** Half-angle of the vision cone, in degrees (45 = a 90-degree cone) */
	UPROPERTY(EditDefaultsOnly, Category="Guard|Vision", meta=(ClampMin="0", ClampMax="180"))
	float PeripheralVisionHalfAngle = 50.f;

	/** How far the guard can hear a reported noise, in cm */
	UPROPERTY(EditDefaultsOnly, Category="Guard|Hearing", meta=(ClampMin="0"))
	float HearingRange = 3000.f;

	/** Seconds spent looking around after losing sight of the player (a hunt) */
	UPROPERTY(EditDefaultsOnly, Category="Guard|Investigate", meta=(ClampMin="0"))
	float LookAroundTime = 3.f;

	/**
	 *  Seconds spent at a heard noise before resuming patrol. Longer than a
	 *  player hunt: a thrown object is a distraction, so the guard should commit
	 *  to it and give the player a real window to move.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Guard|Investigate", meta=(ClampMin="0"))
	float NoiseInvestigateTime = 7.f;

	/** How close counts as reaching an investigation point, in cm */
	UPROPERTY(EditDefaultsOnly, Category="Guard|Investigate", meta=(ClampMin="0"))
	float InvestigateAcceptanceRadius = 80.f;

	/**
	 *  Extra distance, in cm, the guard walks past the point of interest along
	 *  its approach, so it steps into the room instead of stopping in the
	 *  doorway where a wall would clip its view.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Guard|Investigate", meta=(ClampMin="0"))
	float InvestigateOvershoot = 200.f;

	/**
	 *  Vision cone half-angle while looking around at an investigation point.
	 *  Wider than the patrol cone so the guard surveys the whole area at once
	 *  without turning. A future investigation animation plays on top of this.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Guard|Investigate", meta=(ClampMin="0", ClampMax="180"))
	float InvestigateVisionHalfAngle = 160.f;

	/** Draw guard state and investigation targets while testing */
	UPROPERTY(EditDefaultsOnly, Category="Guard|Debug")
	bool bDebugGuard = true;

private:

	// --- State entry ---
	void EnterPatrol();
	void EnterInvestigate(const FVector& PointOfInterest, bool bFromNoise);
	void EnterAlerted();

	/** On-screen state + investigation-target draw for debugging */
	void DrawGuardDebug(const FVector& InvestigatePoint, bool bFromNoise) const;

	// --- Patrol helpers ---
	void MoveToCurrentPatrolPoint();
	void AdvancePatrol();

	// --- Look-around at an investigation point ---
	/** Widen the vision cone, hold for LookAroundTime, then resume patrol */
	void StartLookAround();

	/** Set the sight cone to the wide investigate angle or back to the patrol angle */
	void SetVisionWide(bool bWide);

	// --- Perception ---
	UFUNCTION()
	void HandlePerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	void OnSawPlayer(APawn* Player);
	void OnLostPlayer();
	void OnHeardNoise(const FVector& NoiseLocation);

	/** Configure the sense configs from the tuning properties */
	void ConfigureSenses();

	/** Tell the detection subsystem whether this guard sees the player */
	void ReportSeesPlayer(bool bSeesPlayer);

	/** Clear any pending look-around / wait timer */
	void ClearWaitTimer();

	UPROPERTY(Transient)
	TObjectPtr<AHeistGuardCharacter> GuardPawn;

	UPROPERTY(Transient)
	TObjectPtr<UAIPerceptionComponent> GuardPerception;

	UPROPERTY(Transient)
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(Transient)
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	/** Cached detection meter, resolved in BeginPlay */
	UPROPERTY(Transient)
	TObjectPtr<UHeistDetectionSubsystem> DetectionSubsystem;

	/** The player pawn while sensed */
	TWeakObjectPtr<APawn> SeenPlayer;

	EGuardState GuardState = EGuardState::Patrol;

	int32 PatrolIndex = 0;
	int32 PatrolDirection = 1;

	/**
	 *  True while the current investigation is a player hunt (lost sight of the
	 *  player) rather than a heard noise. A player hunt outranks noises, so a
	 *  thrown object cannot pull the guard off the player's trail.
	 */
	bool bInvestigatingPlayer = false;

	FTimerHandle WaitTimerHandle;
};

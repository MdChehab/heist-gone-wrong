// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HeistGuardCharacter.generated.h"

class USoundBase;
class UAudioComponent;
class UAnimMontage;

/**
 *  The guard's body: mesh, movement and the patrol route data. The behaviour
 *  (deciding where to go and when) lives in AHeistGuardController, so this class
 *  only holds the designer-facing configuration and exposes it through getters.
 */
UCLASS(abstract)
class AHeistGuardCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	AHeistGuardCharacter();

	/** Ordered patrol waypoints. Assign placed marker actors (e.g. TargetPoints) per guard. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Guard|Patrol")
	TArray<TObjectPtr<AActor>> PatrolPoints;

	/**
	 *  While alerted the guard stands still and must face the player, so it
	 *  follows the controller's (focus) rotation instead of its movement
	 *  direction. Patrol/investigate switch it back to orient-to-movement.
	 */
	void SetFaceControlRotation(bool bEnabled);

	/** Teleport back to the spawn transform for a checkpoint restart */
	void ResetToStart();

	/** Set the movement speed for the current behaviour state */
	void ApplyPatrolSpeed();
	void ApplyInvestigateSpeed();
	void ApplyChaseSpeed();

	/** Bark a random alert line when the guard spots the player (called on Alerted) */
	void PlayAlertBark();

	/** Bark a random suspicious line when the guard investigates (called on Investigate) */
	void PlaySuspicionBark();

	/** Play the look-around animation while surveying an investigation point */
	void PlayInvestigateMontage();

	const TArray<TObjectPtr<AActor>>& GetPatrolPoints() const { return PatrolPoints; }
	FORCEINLINE float GetWaitTimeAtPoint() const { return WaitTimeAtPoint; }
	FORCEINLINE bool ShouldLoopPatrol() const { return bLoopPatrol; }
	FORCEINLINE float GetPatrolAcceptanceRadius() const { return PatrolAcceptanceRadius; }

protected:

	/** Apply PatrolSpeed so Blueprint-tuned values reach the movement component */
	virtual void BeginPlay() override;

	/** Seconds the guard pauses at each waypoint before moving on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard|Patrol", meta=(ClampMin="0", UIMin="0"))
	float WaitTimeAtPoint = 2.f;

	/** Loop back to the first point after the last (true), or reverse along the route (false) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard|Patrol")
	bool bLoopPatrol = true;

	/** How close, in cm, counts as having reached a waypoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard|Patrol", meta=(ClampMin="0", UIMin="0"))
	float PatrolAcceptanceRadius = 60.f;

	/** Patrol walk speed (slow) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard|Movement", meta=(ClampMin="0", UIMin="0"))
	float PatrolSpeed = 200.f;

	/** Move speed heading to a noise / last-known spot (a jog) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard|Movement", meta=(ClampMin="0", UIMin="0"))
	float InvestigateSpeed = 380.f;

	/** Move speed approaching a spotted player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard|Movement", meta=(ClampMin="0", UIMin="0"))
	float ChaseSpeed = 460.f;

	/** Lines barked when the guard spots the player (a random one plays). E.g. "Intruder!" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard|Audio")
	TArray<TObjectPtr<USoundBase>> AlertBarks;

	/** Lines barked when the guard gets suspicious and investigates. E.g. "Who's there?" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard|Audio")
	TArray<TObjectPtr<USoundBase>> SuspicionBarks;

	/** Look-around animation played while surveying an investigation point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard|Animation")
	TObjectPtr<UAnimMontage> InvestigateMontage;

private:

	/** Pick a random line from Barks and play it, cutting off any bark still playing */
	void PlayBark(const TArray<TObjectPtr<USoundBase>>& Barks);

	/** The bark currently playing on this guard, so a new one can interrupt it */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ActiveBark;

private:

	/** Spawn transform, captured in BeginPlay, restored on a checkpoint restart */
	FTransform StartTransform;
};

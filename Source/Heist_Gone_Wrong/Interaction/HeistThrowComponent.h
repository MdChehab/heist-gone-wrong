// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeistThrowComponent.generated.h"

class AHeistThrowable;

/** Fired when the carried object changes. Carried is null when nothing is held. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHeistCarriedChanged, AHeistThrowable*, Carried);

/**
 *  Carries a single AHeistThrowable and launches it on request.
 *
 *  The throw is charged: hold to wind up, release to launch. Longer holds throw
 *  further, which is what gives one throwable a range of distraction distances
 *  without needing several throwable types.
 *
 *  Owner-agnostic like the interaction component: it aims using the generic
 *  AActor::GetActorEyesViewPoint and reports state through a delegate, so it
 *  never casts its owner to a concrete class.
 */
UCLASS(ClassGroup=(Heist), meta=(BlueprintSpawnableComponent))
class UHeistThrowComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UHeistThrowComponent();

	/** Take Throwable into hand. Returns false if already carrying something. */
	UFUNCTION(BlueprintCallable, Category="Throw")
	bool Carry(AHeistThrowable* Throwable);

	/** Start winding up a throw. No effect when empty-handed. */
	UFUNCTION(BlueprintCallable, Category="Throw")
	void BeginCharge();

	/** Launch the carried object, scaled by how long the charge was held. */
	UFUNCTION(BlueprintCallable, Category="Throw")
	bool ReleaseCharge();

	/** Abandon a wind-up without throwing (e.g. on stagger or detection). */
	UFUNCTION(BlueprintCallable, Category="Throw")
	void CancelCharge();

	/** 0 to 1 charge progress. Poll this for a HUD power meter. */
	UFUNCTION(BlueprintCallable, Category="Throw")
	float GetChargeRatio() const;

	UFUNCTION(BlueprintCallable, Category="Throw")
	bool IsCharging() const { return bIsCharging; }

	/** Currently carried object, or null */
	UFUNCTION(BlueprintCallable, Category="Throw")
	AHeistThrowable* GetCarried() const { return CarriedThrowable; }

	UFUNCTION(BlueprintCallable, Category="Throw")
	bool IsCarrying() const { return CarriedThrowable != nullptr; }

	/** Bind the HUD to this */
	UPROPERTY(BlueprintAssignable, Category="Throw")
	FHeistCarriedChanged OnCarriedChanged;

protected:

	virtual void BeginPlay() override;

	/**
	 *  Socket or bone on the owner's skeletal mesh to attach the carried object
	 *  to. A name the skeleton does not have makes the object snap to the mesh
	 *  origin instead of the hand, so BeginPlay warns when it cannot be found.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw")
	FName CarrySocket = TEXT("hand_r");

	/** Local offset from the socket, to seat the object in the hand */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw")
	FVector CarryLocationOffset = FVector::ZeroVector;

	/** Local rotation from the socket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw")
	FRotator CarryRotationOffset = FRotator::ZeroRotator;

	/** Launch speed for a tapped throw, in cm/s. Low enough to read as a drop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw|Charge", meta=(ClampMin="0", UIMin="0"))
	float MinThrowSpeed = 250.f;

	/** Launch speed at full charge, in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw|Charge", meta=(ClampMin="0", UIMin="0"))
	float MaxThrowSpeed = 1700.f;

	/** Seconds of holding needed to reach MaxThrowSpeed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw|Charge", meta=(ClampMin="0.01", UIMin="0.01"))
	float MaxChargeTime = 1.1f;

	/**
	 *  How far in front of the view the object is placed before physics wakes.
	 *  Must exceed the owner's capsule radius (42 on the default character) or
	 *  the throw starts inside their collision and is ejected at random.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw", meta=(ClampMin="0", UIMin="0"))
	float MuzzleOffset = 100.f;

	/** Upward tilt added to the view direction, in degrees, to give the throw an arc */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw", meta=(ClampMin="-89", ClampMax="89"))
	float ThrowPitchOffset = 10.f;

	/** Log throw vectors and draw the intended path. Diagnostic only. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw|Debug")
	bool bDebugThrow = false;

private:

	/** Shared launch path used by ReleaseCharge */
	bool LaunchCarried(float Speed);

	/**
	 *  Strong reference on purpose: while carried, this component is the only
	 *  thing keeping the throwable referenced, so it must not be collected.
	 */
	UPROPERTY(Transient)
	TObjectPtr<AHeistThrowable> CarriedThrowable;

	/**
	 *  Resolved once in BeginPlay: the owner's skeletal mesh if it has one, so
	 *  CarrySocket can name a hand bone, otherwise the owner's root component.
	 */
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> CachedAttachParent;

	/** True between BeginCharge and ReleaseCharge/CancelCharge */
	bool bIsCharging = false;

	/** World seconds at BeginCharge. Elapsed time is derived, so no Tick. */
	double ChargeStartTime = 0.0;
};

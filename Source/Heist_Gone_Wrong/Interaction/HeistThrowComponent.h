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

	/** Launch the carried object along the owner's view. Returns false if empty-handed. */
	UFUNCTION(BlueprintCallable, Category="Throw")
	bool ThrowCarried();

	/** Currently carried object, or null */
	UFUNCTION(BlueprintCallable, Category="Throw")
	AHeistThrowable* GetCarried() const { return CarriedThrowable; }

	UFUNCTION(BlueprintCallable, Category="Throw")
	bool IsCarrying() const { return CarriedThrowable != nullptr; }

	/** Bind the HUD to this */
	UPROPERTY(BlueprintAssignable, Category="Throw")
	FHeistCarriedChanged OnCarriedChanged;

protected:

	/** Socket on the owner's mesh to attach the carried object to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw")
	FName CarrySocket = TEXT("hand_r");

	/** Launch speed applied on throw */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw", meta=(ClampMin="0", UIMin="0"))
	float ThrowImpulse = 900.f;

	/** Upward tilt added to the view direction, in degrees, to give the throw an arc */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw", meta=(ClampMin="-89", ClampMax="89"))
	float ThrowPitchOffset = 10.f;

private:

	/**
	 *  Strong reference on purpose: while carried, this component is the only
	 *  thing keeping the throwable referenced, so it must not be collected.
	 */
	UPROPERTY(Transient)
	TObjectPtr<AHeistThrowable> CarriedThrowable;
};

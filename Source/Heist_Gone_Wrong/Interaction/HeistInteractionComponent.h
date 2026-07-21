// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeistInteractionComponent.generated.h"

/** Fired when the focused interactable changes. NewFocus is null when focus is lost. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHeistFocusChanged, AActor*, NewFocus, const FText&, Prompt);

/**
 *  Finds the best IHeistInteractable near the owner and tracks it as the
 *  current focus, then runs the interaction on request.
 *
 *  Uses a proximity sphere around the owner rather than a camera-forward trace.
 *  In third person the objects worth grabbing sit on floors and plinths, below
 *  a ray cast forward from eye height, so a trace misses the very cases the
 *  player expects to work (notably standing directly over an object).
 *
 *  Deliberately owner-agnostic: it reads the view point through the generic
 *  AActor::GetActorEyesViewPoint and reports focus changes through a delegate,
 *  so it never casts its owner to a concrete class and stays reusable.
 */
UCLASS(ClassGroup=(Heist), meta=(BlueprintSpawnableComponent))
class UHeistInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UHeistInteractionComponent();

	/** Interact with the focused actor. Returns true if an interaction ran. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool TryInteract();

	/** Bind the HUD prompt to this */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FHeistFocusChanged OnFocusChanged;

	/** Currently focused interactable, or null */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Radius around the owner searched for interactables, in cm */
	UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin="0", UIMin="0"))
	float InteractionRange = 200.f;

	/**
	 *  How much the owner must be facing a candidate, as a dot product.
	 *  1 = dead ahead, 0 = anywhere in the front half, -1 = any direction.
	 */
	UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin="-1", ClampMax="1"))
	float MinFacingDot = 0.f;

	/**
	 *  Inside this distance the facing test is skipped entirely. Standing on
	 *  top of an object leaves no meaningful horizontal direction to it, so
	 *  requiring the player to face it would make close pickups fail.
	 */
	UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin="0", UIMin="0"))
	float FacingIgnoreDistance = 90.f;

	/** Block interaction through walls */
	UPROPERTY(EditAnywhere, Category="Interaction")
	bool bRequireLineOfSight = true;

	/** Seconds between focus scans. Kept off Tick on purpose. */
	UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin="0.01", UIMin="0.01"))
	float ScanInterval = 0.15f;

private:

	/** Timer callback: pick the best interactable near the owner */
	void ScanForInteractable();

	/** Whether Candidate passes the facing and line-of-sight filters */
	bool IsCandidateReachable(const AActor* Candidate, const FVector& OwnerLocation,
		const FVector& ViewLocation, const FVector& OwnerForward) const;

	/** Swap focus and broadcast, ignoring no-op changes */
	void SetFocus(AActor* NewFocus);

	/**
	 *  Weak on purpose: focus must never keep a destroyed or streamed-out actor
	 *  alive, so this is intentionally not a UPROPERTY/TObjectPtr.
	 */
	TWeakObjectPtr<AActor> FocusedActor;

	FTimerHandle ScanTimerHandle;
};

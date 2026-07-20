// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeistInteractionComponent.generated.h"

/** Fired when the focused interactable changes. NewFocus is null when focus is lost. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHeistFocusChanged, AActor*, NewFocus, const FText&, Prompt);

/**
 *  Scans in front of the owner for the nearest IHeistInteractable and tracks it
 *  as the current focus, then runs the interaction on request.
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

	/** How far in front of the view the scan reaches */
	UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin="0", UIMin="0"))
	float InteractionRange = 250.f;

	/** Sweep radius. Larger is more forgiving to aim. */
	UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin="0", UIMin="0"))
	float InteractionRadius = 40.f;

	/** Seconds between focus scans. Kept off Tick on purpose. */
	UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin="0.01", UIMin="0.01"))
	float ScanInterval = 0.15f;

private:

	/** Timer callback: find the best interactable in front of the owner */
	void ScanForInteractable();

	/** Swap focus and broadcast, ignoring no-op changes */
	void SetFocus(AActor* NewFocus);

	/**
	 *  Weak on purpose: focus must never keep a destroyed or streamed-out actor
	 *  alive, so this is intentionally not a UPROPERTY/TObjectPtr.
	 */
	TWeakObjectPtr<AActor> FocusedActor;

	FTimerHandle ScanTimerHandle;
};

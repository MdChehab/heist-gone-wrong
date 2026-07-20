// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/HeistInteractable.h"
#include "HeistThrowable.generated.h"

class UStaticMeshComponent;

/**
 *  A pickup-and-throw object. Picking it up is an interaction; landing after a
 *  throw reports a hearing-sense noise event so guards can investigate it (W4).
 */
UCLASS(abstract)
class AHeistThrowable : public AActor, public IHeistInteractable
{
	GENERATED_BODY()

public:

	AHeistThrowable();

	//~ Begin IHeistInteractable
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	//~ End IHeistInteractable

	/** Attach to Carrier and stop simulating physics */
	UFUNCTION(BlueprintCallable, Category="Throwable")
	void PickUp(AActor* Carrier, FName AttachSocket);

	/** Detach, re-enable physics and launch along Direction */
	UFUNCTION(BlueprintCallable, Category="Throwable")
	void Throw(const FVector& Direction, float Impulse);

	/** True while attached to a carrier */
	UFUNCTION(BlueprintCallable, Category="Throwable")
	bool IsHeld() const { return bIsHeld; }

protected:

	/** Binds the impact handler used to report throw noise */
	virtual void BeginPlay() override;

	/** Visible body and physics collider */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Prompt shown when this is in focus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable")
	FText InteractionPrompt;

	/** How loud the landing is, in world units of hearing radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable", meta=(ClampMin="0", UIMin="0"))
	float NoiseRange = 1500.f;

	/** Scales the reported noise loudness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable", meta=(ClampMin="0", UIMin="0"))
	float NoiseLoudness = 1.f;

	/** Ignore landings gentler than this so a nudge does not alert anyone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable", meta=(ClampMin="0", UIMin="0"))
	float MinImpactSpeedForNoise = 250.f;

	/** Reports the noise event guards listen for. Override to add audio/VFX. */
	UFUNCTION(BlueprintNativeEvent, Category="Throwable")
	void ReportNoise(const FVector& Location);

	UFUNCTION()
	void HandleMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:

	/** True from PickUp until Throw */
	UPROPERTY(Transient)
	bool bIsHeld = false;

	/** Set on Throw, cleared on the first qualifying impact, so one throw makes one noise */
	UPROPERTY(Transient)
	bool bAwaitingImpactNoise = false;

	/** Who threw it, so guards can be told the instigator later */
	TWeakObjectPtr<AActor> LastThrower;
};

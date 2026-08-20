// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/HeistInteractable.h"
#include "HeistSwitch.generated.h"

class UStaticMeshComponent;
class USoundBase;

/**
 *  A switch the player interacts with to drive its linked activatables (the
 *  door). It talks to targets only through IHeistActivatable, so it never needs
 *  to know they are doors. One-shot by default, or a toggle.
 */
UCLASS()
class AHeistSwitch : public AActor, public IHeistInteractable
{
	GENERATED_BODY()

public:

	AHeistSwitch();

	//~ Begin IHeistInteractable
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	//~ End

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> SwitchMesh;

	/** Prompt shown when the switch is in focus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch")
	FText InteractionPrompt;

	/** Actors implementing IHeistActivatable that this switch drives (assign the door) */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Switch")
	TArray<TObjectPtr<AActor>> Targets;

	/** Toggle the targets on each use; otherwise it is a one-shot activate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch")
	bool bToggle = false;

	/** Played when the switch is used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Switch|Audio")
	TObjectPtr<USoundBase> ActivateSound;

private:

	void SetTargetsActive(bool bActive, AActor* Activator);

	/** Restore the banked state on a checkpoint restart */
	UFUNCTION()
	void HandleRunReset();

	/** Bank the current state when a checkpoint is reached */
	UFUNCTION()
	void HandleCheckpointSaved();

	bool bActivated = false;

	/** Activated state banked at the last checkpoint */
	bool bSavedActivated = false;
};

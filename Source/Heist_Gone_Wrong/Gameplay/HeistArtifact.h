// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/HeistInteractable.h"
#include "HeistArtifact.generated.h"

class UStaticMeshComponent;
class UHeistObjectiveSubsystem;
class USoundBase;

/**
 *  The stealable artifact. Interacting with it takes it (advancing the
 *  objective to "escape") and removes it from the world. A checkpoint restart
 *  puts it back so the run can be tried again.
 */
UCLASS()
class AHeistArtifact : public AActor, public IHeistInteractable
{
	GENERATED_BODY()

public:

	AHeistArtifact();

	//~ Begin IHeistInteractable
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	//~ End

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> ArtifactMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Artifact")
	FText InteractionPrompt;

	/** Played when the artifact is taken */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Artifact")
	TObjectPtr<USoundBase> PickupSound;

private:

	/** Hide/show and enable/disable so the artifact leaves and returns to the world */
	void SetTaken(bool bTaken);

	UFUNCTION()
	void HandleRunReset();

	UFUNCTION()
	void HandleCheckpointSaved();

	UPROPERTY(Transient)
	TObjectPtr<UHeistObjectiveSubsystem> Objective;

	bool bTaken = false;

	/** Taken state banked at the last checkpoint */
	bool bSavedTaken = false;
};

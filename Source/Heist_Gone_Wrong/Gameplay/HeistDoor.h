// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/HeistActivatable.h"
#include "HeistDoor.generated.h"

class UStaticMeshComponent;
class USoundBase;

/**
 *  A door that slides/swings open when activated (by a switch) and closes when
 *  deactivated. It interpolates only while moving - ticking is enabled during a
 *  transition and switched off once it settles. On a checkpoint restart it snaps
 *  back to its starting state.
 */
UCLASS()
class AHeistDoor : public AActor, public IHeistActivatable
{
	GENERATED_BODY()

public:

	AHeistDoor();

	//~ Begin IHeistActivatable
	virtual void Activate_Implementation(AActor* Activator) override;
	virtual void Deactivate_Implementation(AActor* Activator) override;
	//~ End

	UFUNCTION(BlueprintCallable, Category="Door")
	void OpenDoor();

	UFUNCTION(BlueprintCallable, Category="Door")
	void CloseDoor();

	UFUNCTION(BlueprintCallable, Category="Door")
	bool IsOpen() const { return bTargetOpen; }

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> DoorRoot;

	/** The moving door panel */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	/** Relative location the panel moves to when open (e.g. up or aside) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door")
	FVector OpenLocationOffset = FVector(0.f, 0.f, 250.f);

	/** Relative rotation the panel takes when open (e.g. a 90-degree swing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door")
	FRotator OpenRotationOffset = FRotator::ZeroRotator;

	/** Seconds to fully open or close */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door", meta=(ClampMin="0.01", UIMin="0.01"))
	float OpenDuration = 1.2f;

	/** Whether the door begins open */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door")
	bool bStartsOpen = false;

	/** Played when the door starts opening */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Audio")
	TObjectPtr<USoundBase> OpenSound;

	/** Played when the door starts closing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Audio")
	TObjectPtr<USoundBase> CloseSound;

private:

	void SetTargetOpen(bool bOpen);
	void ApplyAlpha(float Alpha);

	/** Snap back to the banked state on a checkpoint restart */
	UFUNCTION()
	void HandleRunReset();

	/** Bank the current open/closed state when a checkpoint is reached */
	UFUNCTION()
	void HandleCheckpointSaved();

	/** Closed pose, captured from the panel's editor-set relative transform */
	FVector ClosedRelLocation = FVector::ZeroVector;
	FRotator ClosedRelRotation = FRotator::ZeroRotator;

	bool bTargetOpen = false;

	/** Open state banked at the last checkpoint */
	bool bSavedOpen = false;

	/** 0 = closed, 1 = open */
	float OpenAlpha = 0.f;
};

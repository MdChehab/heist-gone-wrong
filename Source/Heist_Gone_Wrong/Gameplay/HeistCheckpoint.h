// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeistCheckpoint.generated.h"

class UBoxComponent;
class UHeistObjectiveSubsystem;

/**
 *  Passing through this sets the run's respawn point to this actor's transform,
 *  so a later failure sends the player here instead of all the way to the start.
 *  Place it (facing the way the player should respawn) past a milestone.
 */
UCLASS()
class AHeistCheckpoint : public AActor
{
	GENERATED_BODY()

public:

	AHeistCheckpoint();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> Trigger;

private:

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(Transient)
	TObjectPtr<UHeistObjectiveSubsystem> Objective;

	bool bReached = false;
};

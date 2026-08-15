// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeistExitVolume.generated.h"

class UBoxComponent;
class UHeistObjectiveSubsystem;

/**
 *  The escape point. When the player enters carrying the artifact, the run is
 *  won. Entering without it does nothing (with a hint while testing).
 */
UCLASS()
class AHeistExitVolume : public AActor
{
	GENERATED_BODY()

public:

	AHeistExitVolume();

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
};

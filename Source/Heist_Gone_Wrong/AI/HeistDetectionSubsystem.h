// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HeistDetectionSubsystem.generated.h"

/** Broadcasts the current 0..1 detection level whenever it changes (bind the HUD meter). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHeistDetectionChanged, float, DetectionLevel);

/** Fires once when the meter fills. W5 hooks this to fail + checkpoint restart. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHeistPlayerDetected);

/**
 *  The single, game-wide detection meter. Guards report whether they currently
 *  see the player; the meter fills while any guard does and drains otherwise.
 *
 *  This is a world subsystem rather than an actor component because it is shared
 *  state that many guards write to and the HUD reads from - one meter, one owner,
 *  no casting between guards and the player.
 */
UCLASS()
class UHeistDetectionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	/** A guard reports that it currently sees / no longer sees the player. */
	UFUNCTION(BlueprintCallable, Category="Detection")
	void SetGuardSeesPlayer(AActor* Guard, bool bSeesPlayer);

	UFUNCTION(BlueprintCallable, Category="Detection")
	float GetDetectionLevel() const { return DetectionLevel; }

	UFUNCTION(BlueprintCallable, Category="Detection")
	bool IsAnyGuardSeeingPlayer() const;

	/** Clear the meter and seeing-guard set (e.g. on checkpoint respawn in W5). */
	UFUNCTION(BlueprintCallable, Category="Detection")
	void ResetDetection();

	/** Bind the HUD meter to this */
	UPROPERTY(BlueprintAssignable, Category="Detection")
	FHeistDetectionChanged OnDetectionChanged;

	/** Bind the fail/restart handler to this (W5) */
	UPROPERTY(BlueprintAssignable, Category="Detection")
	FHeistPlayerDetected OnPlayerDetected;

	/** Meter fraction gained per second while at least one guard sees the player */
	UPROPERTY(BlueprintReadWrite, Category="Detection", meta=(ClampMin="0", UIMin="0"))
	float FillRatePerSecond = 0.6f;

	/** Meter fraction lost per second while unseen */
	UPROPERTY(BlueprintReadWrite, Category="Detection", meta=(ClampMin="0", UIMin="0"))
	float DrainRatePerSecond = 0.35f;

	/** Draw the meter on screen while testing */
	UPROPERTY(BlueprintReadWrite, Category="Detection")
	bool bDebugMeter = true;

	//~ Begin USubsystem
	virtual void Deinitialize() override;
	//~ End USubsystem

private:

	/** Timer callback: fill or drain, broadcast, and fire detection at full */
	void UpdateDetection();

	/** Start the update timer if it should be running and is not */
	void EnsureTimerRunning();

	/** Guards currently seeing the player. Weak so a destroyed guard drops out. */
	TSet<TWeakObjectPtr<AActor>> SeeingGuards;

	float DetectionLevel = 0.f;

	/** Latched so OnPlayerDetected fires once per fill, not every tick at full */
	bool bDetectionFired = false;

	FTimerHandle UpdateTimerHandle;

	static constexpr float UpdateInterval = 0.1f;
};

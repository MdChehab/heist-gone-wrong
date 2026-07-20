// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Heist_Gone_WrongCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UHeistInteractionComponent;
class UHeistThrowComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AHeist_Gone_WrongCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Finds and activates interactables in front of the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHeistInteractionComponent> InteractionComponent;

	/** Carries and launches a picked-up throwable */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHeistThrowComponent> ThrowComponent;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Run Input Action (hold to run) */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* RunAction;

	/** Crouch Input Action (press to toggle crouch) */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* CrouchAction;

	/** Interact Input Action (press to use the focused interactable) */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InteractAction;

	/** Throw Input Action (press to launch the carried object) */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ThrowAction;

	/** Roll Input Action (press to roll for a quick reposition) */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* RollAction;

protected:

	/** Ground speed at the default walking pace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Stealth", meta=(ClampMin="0", UIMin="0"))
	float WalkSpeed = 400.f;

	/** Ground speed while the run input is held */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Stealth", meta=(ClampMin="0", UIMin="0"))
	float RunSpeed = 650.f;

	/** Ground speed while crouched (quieter, slower) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Stealth", meta=(ClampMin="0", UIMin="0"))
	float CrouchSpeed = 200.f;

	/** True while the run input is held. Exposed for the anim blueprint. */
	UPROPERTY(BlueprintReadOnly, Category="Movement|Stealth")
	bool bIsRunning = false;

	/** Launch speed of the roll */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Roll", meta=(ClampMin="0", UIMin="0"))
	float RollSpeed = 900.f;

	/** How long a roll lasts before another can start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Roll", meta=(ClampMin="0.01", UIMin="0.01"))
	float RollDuration = 0.5f;

	/** True while a roll is in progress. Exposed for the anim blueprint. */
	UPROPERTY(BlueprintReadOnly, Category="Movement|Roll")
	bool bIsRolling = false;

protected:

	/** Constructor */
	AHeist_Gone_WrongCharacter();

protected:

	/** Apply designer-tuned movement speeds (constructor values may be overridden in the Blueprint) */
	virtual void BeginPlay() override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Begins running: raises max ground speed to RunSpeed */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStartRun();

	/** Stops running: restores max ground speed to WalkSpeed */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStopRun();

	/** Toggles crouch on or off */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoToggleCrouch();

	/** Uses whatever the interaction component currently has in focus */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoInteract();

	/** Launches the carried throwable, if any */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoThrow();

	/** Rolls in the current movement direction for a quick reposition */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoRoll();

private:

	/** Timer callback that ends the roll lockout */
	void EndRoll();

	FTimerHandle RollTimerHandle;

public:

	/** Returns true while the run input is held */
	FORCEINLINE bool IsRunning() const { return bIsRunning; }

	/** Returns true while a roll is in progress */
	FORCEINLINE bool IsRolling() const { return bIsRolling; }

	/** Returns the interaction component **/
	FORCEINLINE UHeistInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	/** Returns the throw component **/
	FORCEINLINE UHeistThrowComponent* GetThrowComponent() const { return ThrowComponent; }

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};


// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Heist_Gone_WrongCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
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

public:

	/** Returns true while the run input is held */
	FORCEINLINE bool IsRunning() const { return bIsRunning; }

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};


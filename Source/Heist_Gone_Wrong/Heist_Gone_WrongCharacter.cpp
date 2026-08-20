// Copyright Epic Games, Inc. All Rights Reserved.

#include "Heist_Gone_WrongCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Interaction/HeistInteractionComponent.h"
#include "Interaction/HeistThrowComponent.h"
#include "Interaction/HeistThrowable.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "Heist_Gone_Wrong.h"

AHeist_Gone_WrongCharacter::AHeist_Gone_WrongCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Allow crouching (stealth movement). The capsule shrink and crouched
	// speed cap are handled by the engine's built-in crouch support.
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Interaction scanning (throwables, switch, door, artifact)
	InteractionComponent = CreateDefaultSubobject<UHeistInteractionComponent>(TEXT("InteractionComponent"));

	// Carrying and launching throwables
	ThrowComponent = CreateDefaultSubobject<UHeistThrowComponent>(TEXT("ThrowComponent"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AHeist_Gone_WrongCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Re-apply speeds here so values tuned on the Blueprint (which override the
	// constructor defaults) reach the movement component at runtime.
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	// Remember what the roll has to put back.
	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultBrakingDecelerationWalking = GetCharacterMovement()->BrakingDecelerationWalking;

	// Footstep noise runs on a light timer rather than Tick.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FootstepNoiseTimerHandle, this, &AHeist_Gone_WrongCharacter::EmitFootstepNoise,
			FootstepNoiseInterval, /*bLoop*/ true);
	}
}

void AHeist_Gone_WrongCharacter::EmitFootstepNoise()
{
	// Silent unless actually moving on the ground.
	if (GetVelocity().Size2D() < 10.f || GetCharacterMovement()->IsFalling())
	{
		return;
	}

	const bool bCrouched = GetCharacterMovement()->IsCrouching();

	// Footstep SOUND (player audio feedback) plays whenever moving on foot -
	// quieter when crouching, louder when running. This is separate from the
	// guard-hearing noise below, which only a run produces: the player still
	// hears their crouch-steps, but guards do not.
	if (FootstepSounds.Num() > 0)
	{
		// A random variation and a random pitch each step, so it does not sound
		// like the same click on a loop.
		USoundBase* Step = FootstepSounds[FMath::RandRange(0, FootstepSounds.Num() - 1)];
		if (Step)
		{
			const float StateScale = bCrouched ? 0.4f : (bIsRunning ? 1.f : 0.7f);
			const float Volume = FootstepVolume * StateScale;
			const float Pitch = FMath::FRandRange(FootstepPitchMin, FootstepPitchMax);
			UGameplayStatics::PlaySoundAtLocation(this, Step, GetActorLocation(), Volume, Pitch);
		}
	}

	// Crouching is always silent; walking is silent by default (WalkNoiseRange 0);
	// running is loud. Guards hear this through the same AI hearing sense that
	// picks up thrown objects, so the investigate behaviour is already wired.
	float NoiseRange = 0.f;
	if (bCrouched)
	{
		NoiseRange = 0.f;
	}
	else if (bIsRunning)
	{
		NoiseRange = RunNoiseRange;
	}
	else
	{
		NoiseRange = WalkNoiseRange;
	}

	if (NoiseRange <= 0.f)
	{
		return;
	}

	UAISense_Hearing::ReportNoiseEvent(
		this, GetActorLocation(), FootstepNoiseLoudness, this, NoiseRange, TEXT("Footstep"));
}

void AHeist_Gone_WrongCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHeist_Gone_WrongCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AHeist_Gone_WrongCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHeist_Gone_WrongCharacter::Look);

		// Running (hold)
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AHeist_Gone_WrongCharacter::DoStartRun);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AHeist_Gone_WrongCharacter::DoStopRun);

		// Crouching (toggle)
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AHeist_Gone_WrongCharacter::DoToggleCrouch);

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AHeist_Gone_WrongCharacter::DoInteract);

		// Throwing
		// Throwing: hold to charge, release to launch
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Started, this, &AHeist_Gone_WrongCharacter::DoThrowStart);
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Completed, this, &AHeist_Gone_WrongCharacter::DoThrowRelease);
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Canceled, this, &AHeist_Gone_WrongCharacter::DoThrowRelease);

		// Rolling
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &AHeist_Gone_WrongCharacter::DoRoll);
	}
	else
	{
		UE_LOG(LogHeist_Gone_Wrong, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AHeist_Gone_WrongCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AHeist_Gone_WrongCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AHeist_Gone_WrongCharacter::DoMove(float Right, float Forward)
{
	// Movement is locked during a roll. The roll drives its own motion with
	// friction disabled, so any added input would not decelerate and the
	// character would slide.
	if (bIsRolling)
	{
		return;
	}

	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AHeist_Gone_WrongCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AHeist_Gone_WrongCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AHeist_Gone_WrongCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AHeist_Gone_WrongCharacter::DoStartRun()
{
	// Raise the standing max speed. While crouched, MaxWalkSpeedCrouched still
	// caps the actual speed, so running has no effect until the player stands.
	bIsRunning = true;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void AHeist_Gone_WrongCharacter::DoStopRun()
{
	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AHeist_Gone_WrongCharacter::DoInteract()
{
	// The component owns the "what is in front of me" logic; the character just
	// forwards the input so the same component can be reused on other actors.
	if (!IsValid(InteractionComponent))
	{
		return;
	}

	// Throwables route through the throw component so they attach to the carry
	// socket and can be launched. The cast lives here, in the concrete character,
	// rather than inside either component, which both stay owner-agnostic.
	if (AHeistThrowable* Throwable = Cast<AHeistThrowable>(InteractionComponent->GetFocusedActor()))
	{
		if (IsValid(ThrowComponent) && ThrowComponent->Carry(Throwable))
		{
			return;
		}
	}

	InteractionComponent->TryInteract();
}

void AHeist_Gone_WrongCharacter::DoThrowStart()
{
	if (IsValid(ThrowComponent))
	{
		ThrowComponent->BeginCharge();
	}
}

void AHeist_Gone_WrongCharacter::DoThrowRelease()
{
	if (!IsValid(ThrowComponent) || !ThrowComponent->IsCharging())
	{
		return;
	}

	// Face the throw before launching. The character orients to its movement
	// direction, not the camera, so without this the object leaves sideways
	// relative to the body and reads as a bug even when the aim is correct.
	if (const AController* OwningController = GetController())
	{
		const FRotator YawOnly(0.f, OwningController->GetControlRotation().Yaw, 0.f);
		SetActorRotation(YawOnly);
	}

	ThrowComponent->ReleaseCharge();

	// Cosmetic throw animation over the top (needs the DefaultSlot in the anim
	// blueprint). The object already launched; refine timing later with a notify.
	if (ThrowMontage)
	{
		PlayAnimMontage(ThrowMontage);
	}
}

void AHeist_Gone_WrongCharacter::DoRoll()
{
	// No rolling mid-roll or in the air.
	if (bIsRolling || GetCharacterMovement()->IsFalling())
	{
		return;
	}

	bIsRolling = true;

	// The roll's movement comes from the montage's ROOT MOTION - the animation
	// moves the character, so motion matches the feet exactly and never slides.
	// No launch and no friction changes. Requires a root-motion roll clip and
	// the anim blueprint's Root Motion Mode set to use montages. Input stays
	// locked (see DoMove) for the whole clip.
	float RollTime = RollDuration;
	if (RollMontage)
	{
		const float MontageLength = PlayAnimMontage(RollMontage);
		if (MontageLength > 0.f)
		{
			RollTime = MontageLength;
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RollTimerHandle, this, &AHeist_Gone_WrongCharacter::EndRoll, RollTime, /*bLoop*/ false);
	}
}

void AHeist_Gone_WrongCharacter::EndRoll()
{
	// Root motion drives the roll, so there is no launch/friction to restore -
	// just re-enable movement input.
	bIsRolling = false;
}

void AHeist_Gone_WrongCharacter::DoToggleCrouch()
{
	// Crouch()/UnCrouch() handle the capsule resize; the crouched speed cap is
	// MaxWalkSpeedCrouched, set from CrouchSpeed.
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

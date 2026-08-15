// Heist Gone Wrong

#include "HeistGuardCharacter.h"
#include "HeistGuardController.h"
#include "GameFramework/CharacterMovementComponent.h"

AHeistGuardCharacter::AHeistGuardCharacter()
{
	// All guard timing is event/timer driven, so no per-frame tick.
	PrimaryActorTick.bCanEverTick = false;

	// A placed guard should get its brain automatically.
	AIControllerClass = AHeistGuardController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Face the direction of travel so the guard turns through corners naturally.
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 320.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
}

void AHeistGuardCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Constructor value may be overridden on the Blueprint; apply it at runtime.
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

	StartTransform = GetActorTransform();
}

void AHeistGuardCharacter::SetFaceControlRotation(bool bEnabled)
{
	bUseControllerRotationYaw = bEnabled;
	GetCharacterMovement()->bOrientRotationToMovement = !bEnabled;
}

void AHeistGuardCharacter::ResetToStart()
{
	GetCharacterMovement()->StopMovementImmediately();
	SetActorTransform(StartTransform, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);
}

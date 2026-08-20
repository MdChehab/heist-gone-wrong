// Heist Gone Wrong

#include "HeistGuardCharacter.h"
#include "HeistGuardController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

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

void AHeistGuardCharacter::ApplyPatrolSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
}

void AHeistGuardCharacter::ApplyInvestigateSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = InvestigateSpeed;
}

void AHeistGuardCharacter::ApplyChaseSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
}

void AHeistGuardCharacter::PlayBark(const TArray<TObjectPtr<USoundBase>>& Barks)
{
	if (Barks.Num() == 0)
	{
		return;
	}

	USoundBase* Line = Barks[FMath::RandRange(0, Barks.Num() - 1)];
	if (Line == nullptr)
	{
		return;
	}

	// Cut off any line still playing so one guard never speaks two at once.
	if (ActiveBark && ActiveBark->IsPlaying())
	{
		ActiveBark->Stop();
	}

	// Spawn attached so the voice follows the guard, and keep the handle so the
	// next bark can interrupt it.
	ActiveBark = UGameplayStatics::SpawnSoundAttached(Line, GetRootComponent());
}

void AHeistGuardCharacter::PlayAlertBark()
{
	PlayBark(AlertBarks);
}

void AHeistGuardCharacter::PlaySuspicionBark()
{
	PlayBark(SuspicionBarks);
}

void AHeistGuardCharacter::PlayInvestigateMontage()
{
	if (InvestigateMontage)
	{
		PlayAnimMontage(InvestigateMontage);
	}
}

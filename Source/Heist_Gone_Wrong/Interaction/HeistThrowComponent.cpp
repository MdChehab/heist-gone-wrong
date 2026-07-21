// Heist Gone Wrong

#include "HeistThrowComponent.h"
#include "HeistThrowable.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Heist_Gone_Wrong.h"

UHeistThrowComponent::UHeistThrowComponent()
{
	// Charge progress is derived from a timestamp, so nothing to update per frame.
	PrimaryComponentTick.bCanEverTick = false;
}

void UHeistThrowComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		// Prefer the skeletal mesh so CarrySocket can name a hand socket or bone.
		// Resolved once here rather than on every pickup.
		CachedAttachParent = Owner->FindComponentByClass<USkeletalMeshComponent>();

		if (CachedAttachParent == nullptr)
		{
			CachedAttachParent = Owner->GetRootComponent();
		}

		// Silent failure here looks like "the object floats at the wrong place",
		// which is hard to trace back to a typo in a socket name, so say so.
		if (IsValid(CachedAttachParent) && !CarrySocket.IsNone()
			&& !CachedAttachParent->DoesSocketExist(CarrySocket))
		{
			UE_LOG(LogHeist_Gone_Wrong, Warning,
				TEXT("%s: CarrySocket '%s' not found on '%s'. Carried objects will snap to that component's origin instead of the hand."),
				*GetNameSafe(Owner), *CarrySocket.ToString(), *GetNameSafe(CachedAttachParent));
		}
	}
}

bool UHeistThrowComponent::Carry(AHeistThrowable* Throwable)
{
	// One object at a time keeps the stealth choice meaningful.
	if (!IsValid(Throwable) || !IsValid(CachedAttachParent) || IsCarrying())
	{
		return false;
	}

	Throwable->PickUp(CachedAttachParent, CarrySocket);

	// Seat it in the hand. Attachment snaps to the socket exactly, which rarely
	// lines up with an arbitrary mesh's pivot, so this is the manual nudge.
	Throwable->SetActorRelativeLocation(CarryLocationOffset);
	Throwable->SetActorRelativeRotation(CarryRotationOffset);

	CarriedThrowable = Throwable;

	OnCarriedChanged.Broadcast(CarriedThrowable);
	return true;
}

void UHeistThrowComponent::BeginCharge()
{
	const UWorld* World = GetWorld();

	if (!IsCarrying() || World == nullptr)
	{
		return;
	}

	bIsCharging = true;
	ChargeStartTime = World->GetTimeSeconds();
}

void UHeistThrowComponent::CancelCharge()
{
	bIsCharging = false;
}

float UHeistThrowComponent::GetChargeRatio() const
{
	const UWorld* World = GetWorld();

	if (!bIsCharging || World == nullptr)
	{
		return 0.f;
	}

	const double Elapsed = World->GetTimeSeconds() - ChargeStartTime;
	return FMath::Clamp(static_cast<float>(Elapsed) / MaxChargeTime, 0.f, 1.f);
}

bool UHeistThrowComponent::ReleaseCharge()
{
	if (!bIsCharging)
	{
		return false;
	}

	// Read the ratio before clearing the flag; GetChargeRatio needs the state.
	const float ChargeRatio = GetChargeRatio();
	bIsCharging = false;

	const float Speed = FMath::Lerp(MinThrowSpeed, MaxThrowSpeed, ChargeRatio);
	return LaunchCarried(Speed);
}

bool UHeistThrowComponent::LaunchCarried(float Speed)
{
	AActor* Owner = GetOwner();

	if (!IsCarrying() || !IsValid(Owner))
	{
		return false;
	}

	// Generic aim: pawns resolve this to the controller's view.
	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	Owner->GetActorEyesViewPoint(ViewLocation, ViewRotation);

	// Tilt up so the object arcs instead of firing flat at the floor.
	//
	// ControlRotation comes back unnormalized: looking slightly down reads as
	// pitch 350, not -10. Clamping that directly sends 350 + offset past 360 and
	// pins it to the ceiling, i.e. straight up. Normalize to [-180, 180] first.
	const float BasePitch = FRotator::NormalizeAxis(ViewRotation.Pitch);
	ViewRotation.Pitch = FMath::Clamp(BasePitch + ThrowPitchOffset, -89.f, 89.f);

	const FVector LaunchDirection = ViewRotation.Vector();

	// Start the throw clear of the owner's own collision.
	const FVector LaunchLocation = ViewLocation + LaunchDirection * MuzzleOffset;

	AHeistThrowable* Thrown = CarriedThrowable;

	// Clear before throwing so any handler that inspects the component during
	// the throw sees an empty hand rather than a stale reference.
	CarriedThrowable = nullptr;

	Thrown->Throw(LaunchLocation, LaunchDirection, Speed);

	if (bDebugThrow)
	{
		UE_LOG(LogHeist_Gone_Wrong, Warning,
			TEXT("THROW dir=(%s) launchLoc=(%s) speed=%.0f resultVelocity=(%s)"),
			*LaunchDirection.ToCompactString(), *LaunchLocation.ToCompactString(),
			Speed, *Thrown->GetVelocity().ToCompactString());

		if (const UWorld* World = GetWorld())
		{
			DrawDebugLine(World, ViewLocation, LaunchLocation + LaunchDirection * 500.f,
				FColor::Green, /*bPersistent*/ false, /*LifeTime*/ 5.f, 0, 3.f);
		}
	}

	OnCarriedChanged.Broadcast(nullptr);
	return true;
}

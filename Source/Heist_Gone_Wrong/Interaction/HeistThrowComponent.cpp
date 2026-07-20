// Heist Gone Wrong

#include "HeistThrowComponent.h"
#include "HeistThrowable.h"
#include "GameFramework/Actor.h"

UHeistThrowComponent::UHeistThrowComponent()
{
	// Carrying is attachment-driven; nothing to update per frame.
	PrimaryComponentTick.bCanEverTick = false;
}

bool UHeistThrowComponent::Carry(AHeistThrowable* Throwable)
{
	AActor* Owner = GetOwner();

	// One object at a time keeps the stealth choice meaningful.
	if (!IsValid(Throwable) || !IsValid(Owner) || IsCarrying())
	{
		return false;
	}

	Throwable->PickUp(Owner, CarrySocket);
	CarriedThrowable = Throwable;

	OnCarriedChanged.Broadcast(CarriedThrowable);
	return true;
}

bool UHeistThrowComponent::ThrowCarried()
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
	ViewRotation.Pitch = FMath::Clamp(ViewRotation.Pitch + ThrowPitchOffset, -89.f, 89.f);

	AHeistThrowable* Thrown = CarriedThrowable;

	// Clear before throwing so any handler that inspects the component during
	// the throw sees an empty hand rather than a stale reference.
	CarriedThrowable = nullptr;

	Thrown->Throw(ViewRotation.Vector(), ThrowImpulse);

	OnCarriedChanged.Broadcast(nullptr);
	return true;
}

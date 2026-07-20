// Heist Gone Wrong

#include "HeistInteractionComponent.h"
#include "HeistInteractable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UHeistInteractionComponent::UHeistInteractionComponent()
{
	// Scanning runs on a timer, so this component never needs to tick.
	PrimaryComponentTick.bCanEverTick = false;
}

void UHeistInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ScanTimerHandle, this, &UHeistInteractionComponent::ScanForInteractable,
			ScanInterval, /*bLoop*/ true);
	}
}

void UHeistInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScanTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UHeistInteractionComponent::ScanForInteractable()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!IsValid(Owner) || World == nullptr)
	{
		return;
	}

	// Generic view point: pawns resolve this to the controller's eyes, which is
	// what we want for a camera-relative reach, without knowing the owner's type.
	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	Owner->GetActorEyesViewPoint(ViewLocation, ViewRotation);

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * InteractionRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(HeistInteractionScan), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(Owner);

	FHitResult Hit;
	const bool bHit = World->SweepSingleByChannel(
		Hit, ViewLocation, TraceEnd, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(InteractionRadius), Params);

	AActor* Candidate = bHit ? Hit.GetActor() : nullptr;

	// Only actors implementing the contract and willing to be used right now count.
	if (IsValid(Candidate) && Candidate->Implements<UHeistInteractable>()
		&& IHeistInteractable::Execute_CanInteract(Candidate, Owner))
	{
		SetFocus(Candidate);
	}
	else
	{
		SetFocus(nullptr);
	}
}

void UHeistInteractionComponent::SetFocus(AActor* NewFocus)
{
	if (FocusedActor.Get() == NewFocus)
	{
		return;
	}

	FocusedActor = NewFocus;

	// Prompt text is only meaningful when something is focused.
	const FText Prompt = (NewFocus != nullptr)
		? IHeistInteractable::Execute_GetInteractionPrompt(NewFocus)
		: FText::GetEmpty();

	OnFocusChanged.Broadcast(NewFocus, Prompt);
}

bool UHeistInteractionComponent::TryInteract()
{
	AActor* Focus = FocusedActor.Get();
	AActor* Owner = GetOwner();

	if (!IsValid(Focus) || !IsValid(Owner))
	{
		return false;
	}

	// Re-check: the scan is up to ScanInterval stale by the time input arrives.
	if (!Focus->Implements<UHeistInteractable>()
		|| !IHeistInteractable::Execute_CanInteract(Focus, Owner))
	{
		return false;
	}

	IHeistInteractable::Execute_Interact(Focus, Owner);
	return true;
}

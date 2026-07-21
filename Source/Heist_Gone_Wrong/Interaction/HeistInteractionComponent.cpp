// Heist Gone Wrong

#include "HeistInteractionComponent.h"
#include "HeistInteractable.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
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

	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector OwnerForward = Owner->GetActorForwardVector();

	// Generic view point, used as the eye for the line-of-sight check.
	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	Owner->GetActorEyesViewPoint(ViewLocation, ViewRotation);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(HeistInteractionScan), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(Owner);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(
		Overlaps, OwnerLocation, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(InteractionRange), Params);

	// Nearest valid candidate wins.
	AActor* Best = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();

		// One actor can contribute several overlapping components.
		if (!IsValid(Candidate) || Candidate == Best)
		{
			continue;
		}

		if (!Candidate->Implements<UHeistInteractable>()
			|| !IHeistInteractable::Execute_CanInteract(Candidate, Owner))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(OwnerLocation, Candidate->GetActorLocation());
		if (DistanceSq >= BestDistanceSq)
		{
			continue;
		}

		if (!IsCandidateReachable(Candidate, OwnerLocation, ViewLocation, OwnerForward))
		{
			continue;
		}

		Best = Candidate;
		BestDistanceSq = DistanceSq;
	}

	SetFocus(Best);
}

bool UHeistInteractionComponent::IsCandidateReachable(const AActor* Candidate, const FVector& OwnerLocation,
	const FVector& ViewLocation, const FVector& OwnerForward) const
{
	const FVector CandidateLocation = Candidate->GetActorLocation();

	// Facing test on the horizontal plane only, so an object at the player's
	// feet is not rejected for being "below" them.
	FVector ToCandidate = CandidateLocation - OwnerLocation;
	ToCandidate.Z = 0.f;

	const bool bCloseEnoughToSkipFacing = ToCandidate.SizeSquared()
		<= FMath::Square(FacingIgnoreDistance);

	if (!bCloseEnoughToSkipFacing)
	{
		const FVector Forward2D = FVector(OwnerForward.X, OwnerForward.Y, 0.f).GetSafeNormal();
		if (FVector::DotProduct(Forward2D, ToCandidate.GetSafeNormal()) < MinFacingDot)
		{
			return false;
		}
	}

	if (!bRequireLineOfSight)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return true;
	}

	FCollisionQueryParams LosParams(SCENE_QUERY_STAT(HeistInteractionLos), /*bTraceComplex*/ false);
	LosParams.AddIgnoredActor(GetOwner());
	LosParams.AddIgnoredActor(Candidate);

	// Anything blocking between the eye and the object means it is behind cover.
	FHitResult Blocker;
	const bool bBlocked = World->LineTraceSingleByChannel(
		Blocker, ViewLocation, CandidateLocation, ECC_Visibility, LosParams);

	return !bBlocked;
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

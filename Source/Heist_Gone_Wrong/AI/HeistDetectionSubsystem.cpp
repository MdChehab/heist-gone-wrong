// Heist Gone Wrong

#include "HeistDetectionSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

void UHeistDetectionSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	Super::Deinitialize();
}

bool UHeistDetectionSubsystem::IsAnyGuardSeeingPlayer() const
{
	for (const TWeakObjectPtr<AActor>& Guard : SeeingGuards)
	{
		if (Guard.IsValid())
		{
			return true;
		}
	}
	return false;
}

void UHeistDetectionSubsystem::SetGuardSeesPlayer(AActor* Guard, bool bSeesPlayer)
{
	if (!IsValid(Guard))
	{
		return;
	}

	if (bSeesPlayer)
	{
		SeeingGuards.Add(Guard);
	}
	else
	{
		SeeingGuards.Remove(Guard);
	}

	EnsureTimerRunning();
}

void UHeistDetectionSubsystem::ResetDetection()
{
	SeeingGuards.Reset();
	DetectionLevel = 0.f;
	bDetectionFired = false;
	OnDetectionChanged.Broadcast(DetectionLevel);
}

void UHeistDetectionSubsystem::EnsureTimerRunning()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// The timer only needs to run while there is something to fill or drain.
	const bool bShouldRun = IsAnyGuardSeeingPlayer() || DetectionLevel > 0.f;
	if (bShouldRun && !World->GetTimerManager().IsTimerActive(UpdateTimerHandle))
	{
		World->GetTimerManager().SetTimer(
			UpdateTimerHandle, this, &UHeistDetectionSubsystem::UpdateDetection, UpdateInterval, true);
	}
}

void UHeistDetectionSubsystem::UpdateDetection()
{
	// Drop any guards destroyed since the last update.
	for (auto It = SeeingGuards.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}

	const bool bSeen = SeeingGuards.Num() > 0;
	const float Previous = DetectionLevel;

	if (bSeen)
	{
		DetectionLevel = FMath::Min(1.f, DetectionLevel + FillRatePerSecond * UpdateInterval);
	}
	else
	{
		DetectionLevel = FMath::Max(0.f, DetectionLevel - DrainRatePerSecond * UpdateInterval);
	}

	if (!FMath::IsNearlyEqual(Previous, DetectionLevel))
	{
		OnDetectionChanged.Broadcast(DetectionLevel);
	}

	if (DetectionLevel >= 1.f && !bDetectionFired)
	{
		bDetectionFired = true;
		OnPlayerDetected.Broadcast();
	}

#if !UE_BUILD_SHIPPING
	if (bDebugMeter && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			reinterpret_cast<uint64>(this), UpdateInterval + 0.05f,
			bSeen ? FColor::Red : FColor::Yellow,
			FString::Printf(TEXT("Detection: %3.0f%%%s"), DetectionLevel * 100.f,
				bDetectionFired ? TEXT("  [DETECTED]") : TEXT("")));
	}
#endif

	// Idle: fully drained and nobody watching. Stop until a guard reports again.
	if (!bSeen && DetectionLevel <= 0.f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(UpdateTimerHandle);
		}
	}
}

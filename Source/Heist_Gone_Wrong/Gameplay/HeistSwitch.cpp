// Heist Gone Wrong

#include "HeistSwitch.h"
#include "HeistActivatable.h"
#include "HeistObjectiveSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AHeistSwitch::AHeistSwitch()
{
	PrimaryActorTick.bCanEverTick = false;

	SwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchMesh"));
	SetRootComponent(SwitchMesh);

	InteractionPrompt = NSLOCTEXT("Heist", "SwitchPrompt", "Flip Switch");
}

void AHeistSwitch::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UHeistObjectiveSubsystem* Objective = World->GetSubsystem<UHeistObjectiveSubsystem>())
		{
			Objective->OnRunReset.AddDynamic(this, &AHeistSwitch::HandleRunReset);
			Objective->OnCheckpointSaved.AddDynamic(this, &AHeistSwitch::HandleCheckpointSaved);
		}
	}
}

FText AHeistSwitch::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}

bool AHeistSwitch::CanInteract_Implementation(AActor* Interactor) const
{
	// A one-shot switch cannot be used again once thrown.
	return bToggle || !bActivated;
}

void AHeistSwitch::Interact_Implementation(AActor* Interactor)
{
	if (bToggle)
	{
		bActivated = !bActivated;
		SetTargetsActive(bActivated, Interactor);
	}
	else if (!bActivated)
	{
		bActivated = true;
		SetTargetsActive(true, Interactor);
	}
}

void AHeistSwitch::SetTargetsActive(bool bActive, AActor* Activator)
{
	for (AActor* Target : Targets)
	{
		if (IsValid(Target) && Target->Implements<UHeistActivatable>())
		{
			if (bActive)
			{
				IHeistActivatable::Execute_Activate(Target, Activator);
			}
			else
			{
				IHeistActivatable::Execute_Deactivate(Target, Activator);
			}
		}
	}
}

void AHeistSwitch::HandleRunReset()
{
	// The door restores its own state; the switch matches its banked state.
	bActivated = bSavedActivated;
}

void AHeistSwitch::HandleCheckpointSaved()
{
	bSavedActivated = bActivated;
}

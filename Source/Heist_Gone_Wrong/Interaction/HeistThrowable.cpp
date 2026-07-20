// Heist Gone Wrong

#include "HeistThrowable.h"
#include "Components/StaticMeshComponent.h"
#include "Perception/AISense_Hearing.h"

AHeistThrowable::AHeistThrowable()
{
	// Purely event driven: physics moves it, hit events report the noise.
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

	// Needed for HandleMeshHit to fire while simulating.
	MeshComponent->SetNotifyRigidBodyCollision(true);

	InteractionPrompt = NSLOCTEXT("Heist", "PickUpPrompt", "Pick Up");
}

void AHeistThrowable::BeginPlay()
{
	Super::BeginPlay();

	MeshComponent->OnComponentHit.AddDynamic(this, &AHeistThrowable::HandleMeshHit);
}

FText AHeistThrowable::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}

bool AHeistThrowable::CanInteract_Implementation(AActor* Interactor) const
{
	// Cannot pick up something already in someone's hands.
	return !bIsHeld && IsValid(Interactor);
}

void AHeistThrowable::Interact_Implementation(AActor* Interactor)
{
	// The carrier decides the socket, so the throwable does not need to know
	// anything about the actor picking it up. The throw component handles this
	// in practice; this path keeps the actor usable on its own.
	PickUp(Interactor, NAME_None);
}

void AHeistThrowable::PickUp(AActor* Carrier, FName AttachSocket)
{
	if (!IsValid(Carrier) || bIsHeld)
	{
		return;
	}

	bIsHeld = true;
	bAwaitingImpactNoise = false;

	// Held objects must not fight the carrier's movement.
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AttachToActor(Carrier, FAttachmentTransformRules::SnapToTargetIncludingScale, AttachSocket);
}

void AHeistThrowable::Throw(const FVector& Direction, float Impulse)
{
	if (!bIsHeld)
	{
		return;
	}

	LastThrower = GetAttachParentActor();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	bIsHeld = false;
	bAwaitingImpactNoise = true;

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetSimulatePhysics(true);

	// Ignore the thrower briefly so it does not immediately collide with them.
	MeshComponent->IgnoreActorWhenMoving(LastThrower.Get(), true);

	MeshComponent->AddImpulse(Direction.GetSafeNormal() * Impulse, NAME_None, /*bVelChange*/ true);
}

void AHeistThrowable::HandleMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// One noise per throw, and only for a landing hard enough to be worth hearing.
	if (!bAwaitingImpactNoise)
	{
		return;
	}

	if (MeshComponent->GetComponentVelocity().Size() < MinImpactSpeedForNoise
		&& NormalImpulse.Size() <= 0.f)
	{
		return;
	}

	bAwaitingImpactNoise = false;
	ReportNoise(Hit.ImpactPoint);
}

void AHeistThrowable::ReportNoise_Implementation(const FVector& Location)
{
	// Guards pick this up through AI Perception hearing in W4. Reporting it now
	// means the guard side is pure configuration later, with no changes here.
	UAISense_Hearing::ReportNoiseEvent(
		this, Location, NoiseLoudness, LastThrower.Get(), NoiseRange, TEXT("ThrowableImpact"));
}

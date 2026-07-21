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
	// Fallback path when something picks this up without a throw component:
	// attach to the interactor's root with no socket. UHeistThrowComponent
	// supplies the skeletal mesh and hand socket instead.
	if (IsValid(Interactor))
	{
		PickUp(Interactor->GetRootComponent(), NAME_None);
	}
}

void AHeistThrowable::PickUp(USceneComponent* AttachParent, FName AttachSocket)
{
	if (!IsValid(AttachParent) || bIsHeld)
	{
		return;
	}

	bIsHeld = true;
	bAwaitingImpactNoise = false;

	// Held objects must not fight the carrier's movement.
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AttachToComponent(AttachParent, FAttachmentTransformRules::SnapToTargetIncludingScale, AttachSocket);
}

void AHeistThrowable::Throw(const FVector& LaunchLocation, const FVector& Direction, float Speed)
{
	if (!bIsHeld)
	{
		return;
	}

	LastThrower = GetAttachParentActor();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	bIsHeld = false;
	bAwaitingImpactNoise = true;

	// Move clear of the thrower BEFORE physics wakes up. Enabling simulation
	// while overlapping their capsule makes the solver resolve the penetration
	// by flinging the object in an arbitrary direction.
	SetActorLocation(LaunchLocation, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetSimulatePhysics(true);

	// Set velocity rather than add to it: while carried the body is dragged by
	// the animating hand bone, and that residual velocity would otherwise be
	// added to the launch and send the throw off-course.
	const FVector LaunchVelocity = Direction.GetSafeNormal() * Speed;
	MeshComponent->SetPhysicsLinearVelocity(LaunchVelocity);
	MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
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

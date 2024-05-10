#include "Projectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "FPSCharacter.h"

AProjectile::AProjectile() {
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(12.0f);
	SphereComponent->SetSimulatePhysics(false);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->CanCharacterStepUpOn = ECB_No;
	SphereComponent->SetCollisionProfileName("OverlapAllQueryPhysics");
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnBeginOverlap);
	SphereComponent->IgnoreActorWhenMoving(GetInstigator(), true);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = SphereComponent;
	ProjectileMovementComponent->InitialSpeed = 2400.0f;
	ProjectileMovementComponent->MaxSpeed = 2400.0f;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetGenerateOverlapEvents(false);
	StaticMeshComponent->CanCharacterStepUpOn = ECB_No;
	StaticMeshComponent->SetCollisionProfileName("NoCollision");
	StaticMeshComponent->SetIsReplicated(false);
}

void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectile, Damage);
}

void AProjectile::BeginPlay() {
	Super::BeginPlay();
}

void AProjectile::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit) {
	if (!HasAuthority()) return;
	if (OtherActor == GetInstigator() || OtherActor == GetOwner()) return;
	if (OtherActor->IsA(AProjectileExplosion::StaticClass())) return;
	if (ProjectileExplosionClass) {
		FTransform ProjectileExplosionTransform;
		ProjectileExplosionTransform.SetLocation(Hit.ImpactPoint);
		ProjectileExplosionTransform.SetRotation(FQuat::Identity);
		ProjectileExplosionTransform.SetScale3D(FVector::OneVector);

		AProjectileExplosion* ProjectileExplosion = GetWorld()->SpawnActorDeferred<AProjectileExplosion>(ProjectileExplosionClass, ProjectileExplosionTransform, GetOwner(), GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		ProjectileExplosion->SetDamage(Damage);
		UGameplayStatics::FinishSpawningActor(ProjectileExplosion, ProjectileExplosionTransform);
	}
	Destroy();
}

void AProjectile::SetLocationToMuzzleAndRotationToTargetNetMulticast_Implementation(FVector HitImpactPoint) {
	AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetInstigator());
	AFPSCharacter* ControllerCharacter = Cast<AFPSCharacter>(UGameplayStatics::GetPlayerController(this, 0)->GetPawn());
	if (!FPSCharacter || !ControllerCharacter) return;
	
	AWeapon* Weapon;
	if (FPSCharacter == ControllerCharacter) {
		Weapon = FPSCharacter->GetEquippedWeapon();
	} else {
		Weapon = FPSCharacter->GetThirdPersonEquippedWeapon();
	}

	FVector ProjectileMeshLocation = Weapon->GetMuzzleLocation();
	StaticMeshComponent->SetWorldLocation(ProjectileMeshLocation);
	// ProjectileMeshRotation doesn't need to be updated to reflect the new Location, based on
	// the MuzzleLocation. I didn't find the reason, but it only affects a visual Actor without
	// collision, and it visually works.
}

void AProjectile::DebugDrawHitResult(AActor* OtherActor, const FHitResult& Hit) {
	DrawDebugSphere(GetWorld(), GetActorLocation(), 5.0f, 8, FColor::Red, false, 100.0f);
	DrawDebugSphere(GetWorld(), Hit.Location, 5.0f, 8, FColor::Blue, false, 100.0f);
	DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 5.0f, 8, FColor::Green, false, 100.0f);		
	DrawDebugSphere(GetWorld(), OtherActor->GetActorLocation(), 10.0f, 8, FColor::Black, false, 5.0f);
}

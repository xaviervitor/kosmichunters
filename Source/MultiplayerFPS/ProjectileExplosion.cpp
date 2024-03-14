#include "ProjectileExplosion.h"

#include "Components/SphereComponent.h"
#include "FPSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h" 

AProjectileExplosion::AProjectileExplosion() {
 	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(32.0f);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetCollisionProfileName("OverlapAll");
	SphereComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectileExplosion::OnBeginOverlap);
}

void AProjectileExplosion::BeginPlay() {
	Super::BeginPlay();
	
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AProjectileExplosion::DestroyDelegate, Duration, false);
}

void AProjectileExplosion::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AProjectileExplosion::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit) {
	if (!HasAuthority()) return;
	AFPSCharacter* OtherFPSCharacter = Cast<AFPSCharacter>(OtherActor);
	AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetInstigator());
	if (!FPSCharacter || !OtherFPSCharacter) return;
	if (IgnoreActors.Contains(OtherFPSCharacter)) return;
	IgnoreActors.Add(OtherFPSCharacter);
	FVector ImpulseDirection = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), OtherFPSCharacter->GetActorLocation());
	OtherFPSCharacter->Damage(FPSCharacter, Hit.BoneName, (OtherFPSCharacter == FPSCharacter) ? 0.0f : Damage, ImpulseDamage, ImpulseDirection);
}

void AProjectileExplosion::DestroyDelegate() {
	Destroy();
}
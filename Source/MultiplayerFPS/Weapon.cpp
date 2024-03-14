#include "Weapon.h"

#include "MultiplayerFPS.h"
#include "FPSCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AWeapon::AWeapon() {
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    RootComponent = SceneComponent;
	
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);

	MuzzleSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleSceneComponent"));
	MuzzleSceneComponent->SetupAttachment(RootComponent);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(MuzzleSceneComponent);
	ArrowComponent->ArrowColor = FColor(140, 60, 130, 255);
	ArrowComponent->ArrowSize = 0.33f;
	ArrowComponent->ArrowLength = 50.0f;
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, FPSCharacter);
}

void AWeapon::SetOwner(AActor* InOwner) {
	Super::SetOwner(InOwner);
	FPSCharacter = Cast<AFPSCharacter>(InOwner);
}

TArray<UPrimitiveComponent*> AWeapon::GetWeaponChildren() {
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	
	TArray<USceneComponent*> ChildrenComponents;
	SkeletalMeshComponent->GetChildrenComponents(false, ChildrenComponents);
	for (USceneComponent* ChildComponent : ChildrenComponents) {
		UPrimitiveComponent* ChildPrimitiveComponent = Cast<UPrimitiveComponent>(ChildComponent);
		PrimitiveComponents.Add(ChildPrimitiveComponent);
	}
	
	return PrimitiveComponents;
}

void AWeapon::FireServer_Implementation() {
	Fire();
}

void AWeapon::Fire() {
	if (GetWorldTimerManager().IsTimerActive(FireTimer)) return;
	if (!FPSCharacter->HasWeaponEquipped()) return;
	int32 WeaponAmmo = FPSCharacter->GetEquippedWeaponAmmo();
	if (WeaponAmmo > 0) {
		FPSCharacter->ConsumeEquippedWeaponAmmo();
		FPSCharacter->PlayAnimMontageNetMulticast(FireAnimMontage);
		FPSCharacter->PlayAnimMontageFirstPersonArmsNetMulticast(FireAnimMontage);
			
		GetWorldTimerManager().SetTimer(FireTimer, FireRate, false);

		if (FireMode == EWeaponFireMode::Hitscan) {
			FireHitscan();
		} else if (FireMode == EWeaponFireMode::Projectile) {
			FireProjectile();
		}
		OnFire();
		FPSCharacter->PlaySoundAtLocationNetMulticast(FireSound);
	} else {
		FPSCharacter->PlaySoundAtLocationNetMulticast(NoAmmoSound);
	}
}

FHitResult AWeapon::FPSCharacterCameraLineTrace() {
	FHitResult Hit;
	FVector Start = FPSCharacter->GetCameraLocation();  
	FVector End = FPSCharacter->GetCameraLocation() + FPSCharacter->GetForwardVector() * HitscanDistance;
	ECollisionChannel Channel = ECollisionChannel::ECC_Visibility;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(TArray<AActor*>({FPSCharacter}));
	UWorld* World = GetWorld();
	World->LineTraceSingleByChannel(Hit, Start, End, Channel, QueryParams);
	// DebugDrawLineTrace(Start, End, Hit);
	return Hit;
}

void AWeapon::FireHitscan() {
	FHitResult Hit = FPSCharacterCameraLineTrace();
	
	AFPSCharacter* HitCharacter = Cast<AFPSCharacter>(Hit.GetActor());
	if (HitCharacter) {
		HitCharacter->Damage(FPSCharacter, Hit.BoneName, Damage);
	}
}

void AWeapon::FireProjectile() {
	if (!ProjectileClass) return;
	
	FTransform ProjectileTransform = FTransform::Identity;
	ProjectileTransform.SetLocation(FPSCharacter->GetCameraLocation());
	ProjectileTransform.SetRotation(FPSCharacter->GetCameraRotation().Quaternion());
	
	FHitResult Hit = FPSCharacterCameraLineTrace();
	
	AProjectile* Projectile = GetWorld()->SpawnActorDeferred<AProjectile>(ProjectileClass, ProjectileTransform, this, FPSCharacter, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Projectile->SetLocationToMuzzleAndRotationToTargetNetMulticast(Hit.ImpactPoint);
	Projectile->SetDamage(Damage);
	UGameplayStatics::FinishSpawningActor(Projectile, ProjectileTransform);
}

void AWeapon::RagdollNetMulticast_Implementation() {
	for (UPrimitiveComponent* Child : GetWeaponChildren()) {
		Child->SetCollisionProfileName("Ragdoll");
		Child->SetEnableGravity(true);
		Child->SetSimulatePhysics(true);
	}
}

void AWeapon::SetFirstPersonWeaponNetMulticast_Implementation() {
	for (UPrimitiveComponent* Child : GetWeaponChildren()) {
		Child->SetOnlyOwnerSee(true);
		Child->SetCastShadow(false);
	}
}

void AWeapon::SetThirdPersonWeaponNetMulticast_Implementation() {
	for (UPrimitiveComponent* Child : GetWeaponChildren()) {
		Child->SetOwnerNoSee(true);
		Child->SetCastShadow(true);
		Child->SetCastHiddenShadow(true);
	}
}

void AWeapon::SetWeaponHiddenShadow(bool bHiddenShadow) {
	for (UPrimitiveComponent* Child : GetWeaponChildren()) {
		Child->SetCastHiddenShadow(bHiddenShadow);
	}
}

void AWeapon::DebugDrawLineTrace(FVector Start, FVector End, FHitResult Hit) {
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.0f);
	DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 8.0f, 8, FColor::Black, false, 5.0f);
}
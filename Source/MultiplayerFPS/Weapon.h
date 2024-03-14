#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/NumericLimits.h"
#include "EnumTypes.h"
#include "Projectile.h"

#include "Weapon.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AWeapon : public AActor {
	GENERATED_BODY()
private:
	UPROPERTY(Replicated) class AFPSCharacter* FPSCharacter;
	UPROPERTY(EditAnywhere) class USceneComponent* SceneComponent;
	UPROPERTY(EditAnywhere) class USkeletalMeshComponent* SkeletalMeshComponent;
	UPROPERTY(EditAnywhere) class USceneComponent* MuzzleSceneComponent;

	UPROPERTY(EditAnywhere) FString WeaponName;
	UPROPERTY(EditAnywhere) EWeaponType WeaponType;
	UPROPERTY(EditAnywhere) EAmmoType AmmoType;
	UPROPERTY(EditAnywhere) EWeaponFireMode FireMode;
	UPROPERTY(EditAnywhere) float HitscanDistance = TNumericLimits<float>::Max();
	UPROPERTY(EditAnywhere) float Damage;
	UPROPERTY(EditAnywhere) float FireRate;
	UPROPERTY(EditAnywhere) class UAnimMontage* FireAnimMontage;
	UPROPERTY(EditAnywhere) class USoundBase* FireSound;
	UPROPERTY(EditAnywhere) class USoundBase* NoAmmoSound;
	UPROPERTY(EditAnywhere) FTimerHandle FireTimer;
	
	UPROPERTY(EditAnywhere) TSubclassOf<AProjectile> ProjectileClass;
	
	FHitResult FPSCharacterCameraLineTrace();
protected:
	UPROPERTY(BlueprintReadOnly) class UArrowComponent* ArrowComponent;
	
	TArray<UPrimitiveComponent*> GetWeaponChildren();

	void Fire();
	void FireHitscan();
	void FireProjectile();

	void DebugDrawLineTrace(FVector Start, FVector End, FHitResult Hit);
public:
	AWeapon();

	virtual void SetOwner(AActor* InOwner) override;

	UFUNCTION(Server, Reliable) void FireServer();

	UFUNCTION(NetMulticast, Reliable)
	void RagdollNetMulticast();

	UFUNCTION(NetMulticast, Reliable)
	void SetFirstPersonWeaponNetMulticast();

	UFUNCTION(NetMulticast, Reliable)
	void SetThirdPersonWeaponNetMulticast();

	void SetWeaponHiddenShadow(bool bHiddenShadow);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnFire();

	UFUNCTION(BlueprintPure) FORCEINLINE FString GetName() { return WeaponName; }
	UFUNCTION(BlueprintPure) FORCEINLINE EWeaponType GetWeaponType() { return WeaponType; }
	UFUNCTION(BlueprintPure) FORCEINLINE FVector GetMuzzleLocation() { return MuzzleSceneComponent->GetComponentLocation(); }
	// UFUNCTION(BlueprintPure) FORCEINLINE FRotator GetMuzzleRotation() { return MuzzleSceneComponent->GetComponentRotation(); }
};
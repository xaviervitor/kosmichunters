#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileExplosion.h"

#include "Projectile.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AProjectile : public AActor {
	GENERATED_BODY()	
	
	UPROPERTY(EditAnywhere) class USphereComponent* SphereComponent;	
	UPROPERTY(EditAnywhere) class UStaticMeshComponent* StaticMeshComponent;	
	UPROPERTY(EditAnywhere) class UProjectileMovementComponent* ProjectileMovementComponent;	
	UPROPERTY(EditAnywhere) TSubclassOf<AProjectileExplosion> ProjectileExplosionClass;	
	
	UPROPERTY(Replicated) float Damage;
public:	
	AProjectile();

protected:
	virtual void BeginPlay() override;

	void DebugDrawHitResult(AActor* OtherActor, const FHitResult& Hit);
public:	
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable) void SetLocationToMuzzleAndRotationToTargetNetMulticast(FVector HitImpactPoint);
	FORCEINLINE void SetDamage(float InDamage) { Damage = InDamage; }
};

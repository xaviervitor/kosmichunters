#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Containers/Set.h"

#include "ProjectileExplosion.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AProjectileExplosion : public AActor {
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere) class USphereComponent* SphereComponent;
	
	UPROPERTY(EditAnywhere) float Duration = 0.125f;
	UPROPERTY(EditAnywhere) float ImpulseDamage = 0.0f;
	
	TSet<AActor*> IgnoreActors;
	float Damage;

public:	
	AProjectileExplosion();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	FORCEINLINE void SetDamage(float InDamage) { Damage = InDamage; }

	FORCEINLINE void AddIgnoreActors(AActor* Actor) { IgnoreActors.Add(Actor); }

	UFUNCTION()
	void DestroyDelegate();
};

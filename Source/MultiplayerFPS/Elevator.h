#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Elevator.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AElevator : public AActor {
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere) class UBoxComponent* BoxComponent;
	UPROPERTY(EditAnywhere) class UStaticMeshComponent* StaticMeshComponent;
	
	UPROPERTY(EditAnywhere) TArray<AElevator*> LinkedElevators;
	UPROPERTY(EditAnywhere) TArray<FTransform> ElevatorStops;
	
	UPROPERTY(EditAnywhere) float ElevatorSpeed = 20.0f;
	int CurrentStopIndex = 0;

	void StartElevator();

	void EnableCollision();
	void DisableCollision();
public:
	AElevator();

	UFUNCTION(Server, Reliable) void HandleCollisionServer();

	UFUNCTION() void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);
	UFUNCTION() void StartLinkedElevators();
	UFUNCTION() void StopLinkedElevators();
};

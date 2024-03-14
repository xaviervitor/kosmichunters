#include "Elevator.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Algo/Reverse.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FPSCharacter.h"
#include "ElevatorLatentAction.h"

AElevator::AElevator() {
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetCollisionProfileName("OverlapAll");
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AElevator::OnBeginOverlap);
	RootComponent = BoxComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetGenerateOverlapEvents(false);
	StaticMeshComponent->SetupAttachment(RootComponent);
}

void AElevator::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit) {
	if (HasAuthority() && OtherActor->HasAuthority()) {
		HandleCollisionServer();
	}
}

void AElevator::HandleCollisionServer_Implementation() {
	StartLinkedElevators();
}

void AElevator::StartLinkedElevators() {
	DisableCollision();
	StartElevator();
	for (AElevator* LinkedElevator : LinkedElevators) {
		LinkedElevator->DisableCollision();
		LinkedElevator->StartElevator();
	}
}

void AElevator::StartElevator() {
	if (ElevatorStops.Num() <= 1) return;

	FLatentActionManager& LatentActionManager = GetWorld()->GetLatentActionManager();
	int32 ActionCount = LatentActionManager.GetNumActionsForObject(this); 
	if (ActionCount == 0) {
		FTransform InitialTransform = ElevatorStops[CurrentStopIndex];
		FTransform TargetTransform = ElevatorStops[CurrentStopIndex + 1];
		float InterpolationTime = FVector::Distance(TargetTransform.GetLocation(), InitialTransform.GetLocation()) / ElevatorSpeed;
		
		FLatentActionInfo LatentInfo = FLatentActionInfo(0, 1, *(FString) "StopLinkedElevators", (UObject*) this);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new ElevatorLatentAction(
			 	this, InitialTransform, TargetTransform, InterpolationTime, GetWorld()->DeltaTimeSeconds, LatentInfo));
	
		CurrentStopIndex++;
		if (CurrentStopIndex == ElevatorStops.Num() - 1) {
			Algo::Reverse(ElevatorStops);
			CurrentStopIndex = 0;
		}
	}
}

void AElevator::StopLinkedElevators() {
	EnableCollision();
	for (AElevator* LinkedElevator : LinkedElevators) {
		LinkedElevator->EnableCollision();
	}
}

void AElevator::EnableCollision() {
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AElevator::DisableCollision() {
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
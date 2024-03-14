#include "Ladder.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FPSCharacter.h"

ALadder::ALadder() {
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetCollisionProfileName("OverlapAll");
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ALadder::OnBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ALadder::OnEndOverlap);
}

void ALadder::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit) {
	AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(OtherActor);
	if (!FPSCharacter) return;
	if (!Cast<UCapsuleComponent>(OtherComponent)) return;
	
	UCharacterMovementComponent* MovementComponent = FPSCharacter->GetCharacterMovement();
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying);
}

void ALadder::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex) {
	AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(OtherActor);
	if (!FPSCharacter) return;
	if (!Cast<UCapsuleComponent>(OtherComponent)) return;
	
	UCharacterMovementComponent* MovementComponent = FPSCharacter->GetCharacterMovement();
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
}

#include "AnimNotify_EnableRagdoll.h"

#include "FPSCharacter.h"

void UAnimNotify_EnableRagdoll::Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation) {
    AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(MeshComponent->GetOwner());
    if (FPSCharacter) {
        FPSCharacter->RagdollNetMulticast();
    }
}
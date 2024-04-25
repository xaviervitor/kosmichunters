#include "FPSCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameMode.h"
#include "MultiplayerFPS.h"
#include "FPSGameState.h"
#include "FPSPlayerState.h"
#include "FPSPlayerController.h"

AFPSCharacter::AFPSCharacter() {
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	GetCapsuleComponent()->SetCapsuleRadius(45.0f);

	GetThirdPersonMesh()->SetOwnerNoSee(true);
	GetThirdPersonMesh()->SetCastHiddenShadow(true);
	GetThirdPersonMesh()->SetEnableGravity(false);
	GetThirdPersonMesh()->SetGenerateOverlapEvents(false);
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->bUsePawnControlRotation = true;

	FirstPersonArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArms"));
	FirstPersonArms->SetupAttachment(CameraComponent);
	FirstPersonArms->SetOnlyOwnerSee(true);
	FirstPersonArms->SetCastShadow(false);
	
	FirstPersonBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonBody"));
	FirstPersonBody->SetupAttachment(RootComponent);
	FirstPersonBody->SetOnlyOwnerSee(true);
	FirstPersonBody->SetCastShadow(false);

	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	CharacterMovementComponent->MaxWalkSpeed = 1200.0f;
	CharacterMovementComponent->JumpZVelocity = 460.0f;
	CharacterMovementComponent->AirControl = 0.5f;
	CharacterMovementComponent->BrakingDecelerationFlying = 4096.0f; // Climbing Ladder
	CharacterMovementComponent->BrakingFrictionFactor = 1.0f;
}

void AFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(AFPSCharacter, CameraRotationPitch, COND_SkipOwner);
	
	DOREPLIFETIME_CONDITION(AFPSCharacter, Health, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSCharacter, Armor, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSCharacter, Ammo, COND_OwnerOnly);
	DOREPLIFETIME(AFPSCharacter, EquippedWeaponIndex);
	DOREPLIFETIME(AFPSCharacter, Weapons);
	DOREPLIFETIME(AFPSCharacter, ThirdPersonWeapons);
}

void AFPSCharacter::OnRep_CameraRotationPitch() {
	FRotator CameraRotation = CameraComponent->GetComponentRotation();
	FRotator UpdatedCameraRotation = FRotator(CameraRotationPitch, CameraRotation.Yaw, CameraRotation.Roll);
	CameraComponent->SetWorldRotation(UpdatedCameraRotation);
}

void AFPSCharacter::BeginPlay() {
	Super::BeginPlay();

	if (HasAuthority()) {
		Health = MaxHealth;
		Armor = 0.0f;
		// PlaySoundAtLocationNetMulticast(SpawnSound);
	}

	GameMode = Cast<AMultiplayerFPSGameModeBase>(GetWorld()->GetAuthGameMode());	
}

void AFPSCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	for (AWeapon* Weapon : Weapons) {
		Weapon->Destroy();
	}

	for (AWeapon* ThirdPersonWeapon : ThirdPersonWeapons) {
		ThirdPersonWeapon->Destroy();
	}
}

void AFPSCharacter::Landed(const FHitResult& Hit) {
	Super::Landed(Hit);
	UGameplayStatics::PlaySound2D(GetWorld(), LandSound);
}

void AFPSCharacter::FellOutOfWorld(const UDamageType& DmgType) {
	GameMode->OnFall(GetController());
}

void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent) {
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		UEnhancedInputLocalPlayerSubsystem* EnhancedSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		
		EnhancedSubsystem->AddMappingContext(IMC_Player, 1);
		
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AFPSCharacter::Move);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AFPSCharacter::Look);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AFPSCharacter::JumpCharacter);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Ongoing, this, &AFPSCharacter::ClimbLadder);
		EnhancedInputComponent->BindAction(IA_Walk, ETriggerEvent::Triggered, this, &AFPSCharacter::Walk);
		EnhancedInputComponent->BindAction(IA_Walk, ETriggerEvent::Completed, this, &AFPSCharacter::StopWalking);
		
		EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Triggered, this, &AFPSCharacter::Fire);
		EnhancedInputComponent->BindAction(IA_PreviousWeapon, ETriggerEvent::Triggered, this, &AFPSCharacter::PreviousWeapon);
		EnhancedInputComponent->BindAction(IA_NextWeapon, ETriggerEvent::Triggered, this, &AFPSCharacter::NextWeapon);
		
		EnhancedInputComponent->BindAction(IA_Scoreboard_UI, ETriggerEvent::Triggered, this, &AFPSCharacter::ShowScoreboard);
		EnhancedInputComponent->BindAction(IA_Scoreboard_UI, ETriggerEvent::Completed, this, &AFPSCharacter::HideScoreboard);

		EnhancedInputComponent->BindAction(IA_Pause_UI, ETriggerEvent::Triggered, this, &AFPSCharacter::TogglePause);
	}
}

void AFPSCharacter::Move(const FInputActionValue& Value) {
	const FVector2D InputVector = Value.Get<FVector2D>();
	if (Controller == nullptr || (InputVector.Length() == 0.0f)) return;

	EMovementMode MovementMode = GetCharacterMovement()->MovementMode;
	if (MovementMode == EMovementMode::MOVE_Flying) {
		FVector Forward = GetForwardVector();
		FVector Right = GetRightVector();
		AddMovementInput(Forward, InputVector.X);
		AddMovementInput(Right, InputVector.Y);
	} else {
		FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
		FVector ForwardDirection = YawRotation.Vector();
		FVector RightDirection = UKismetMathLibrary::GetRightVector(YawRotation);
		AddMovementInput(ForwardDirection, InputVector.X);
		AddMovementInput(RightDirection, InputVector.Y);
	}
}

void AFPSCharacter::Look(const FInputActionValue& Value) {
	const FVector2D InputVector = Value.Get<FVector2D>();
	if (InputVector.X != 0.0f) {
		AddControllerYawInput(InputVector.X);
	}
	if (InputVector.Y != 0.0f) {
		AddControllerPitchInput(-InputVector.Y);
		SetControlRotationPitchServer(CameraComponent->GetComponentRotation().Pitch);
	}
}

void AFPSCharacter::JumpCharacter(const FInputActionValue& Value) {
	EMovementMode MovementMode = GetCharacterMovement()->MovementMode;
	// Can't run Jump and ClimbLadder together, because Jump() seems to
	// cancel the jumping action automatically after some time. 
	if (MovementMode == EMovementMode::MOVE_Flying) {
		ClimbLadder(Value);
	} else {
		Jump();
	}
}

void AFPSCharacter::ClimbLadder(const FInputActionValue& Value) {
	FVector UpDirection = FVector(0.0f, 0.0f, 1.0f);
	AddMovementInput(UpDirection);
}

void AFPSCharacter::Walk(const FInputActionValue& Value) {
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;
}

void AFPSCharacter::StopWalking(const FInputActionValue& Value) {
	GetCharacterMovement()->MaxWalkSpeed = 1200.0f;
}

void AFPSCharacter::SetScoreboardVisibility(bool bVisibility) {
	AFPSPlayerController* PlayerController = Cast<AFPSPlayerController>(GetController());
	if (PlayerController) PlayerController->SetScoreboardVisibility(bVisibility);
}

void AFPSCharacter::ShowScoreboard(const FInputActionValue& Value) {
	SetScoreboardVisibility(true);
}

void AFPSCharacter::HideScoreboard(const FInputActionValue& Value) {
	SetScoreboardVisibility(false);
}

void AFPSCharacter::TogglePause(const FInputActionValue& Value) {
	AFPSPlayerController* PlayerController = Cast<AFPSPlayerController>(GetController());
	if (PlayerController) PlayerController->TogglePause();
}

void AFPSCharacter::Fire(const FInputActionValue& Value) {
	AWeapon* EquippedWeapon = GetEquippedWeapon();
	if (EquippedWeapon) {
		EquippedWeapon->FireServer();
	}
}

void AFPSCharacter::PreviousWeapon(const FInputActionValue& Value) {
	PreviousWeaponServer();
}

void AFPSCharacter::NextWeapon(const FInputActionValue& Value) {
	NextWeaponServer();
}

void AFPSCharacter::SetControlRotationPitchServer_Implementation(float InCameraRotationPitch) {
	CameraRotationPitch = InCameraRotationPitch;
	OnRep_CameraRotationPitch(); // The server has to call the OnRep function manually because it is the one sending the replication updates. (https://nerivec.github.io/old-ue4-wiki/pages/network-replication-using-replicatedusing-repnotify-vars.html)
}

void AFPSCharacter::PlayAnimMontageNetMulticast_Implementation(UAnimMontage* AnimMontage) {
	if (AnimMontage) {
		PlayAnimMontage(AnimMontage);
	}
}

void AFPSCharacter::PlayAnimMontageFirstPersonArmsNetMulticast_Implementation(UAnimMontage* AnimMontage) {
	UAnimInstance * AnimInstance = (FirstPersonArms) ? FirstPersonArms->GetAnimInstance() : nullptr; 
	if (AnimMontage && AnimInstance) {
		AnimInstance->Montage_Play(AnimMontage, 1.0f);
	}
}

void AFPSCharacter::PlaySoundAtLocationClient_Implementation(USoundBase* Sound) {
	if (Sound) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
	}
}

void AFPSCharacter::PlaySoundAtLocationNetMulticast_Implementation(USoundBase* Sound) {
	if (Sound) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
	}
}

void AFPSCharacter::AddWeaponServer_Implementation(TSubclassOf<AWeapon> WeaponClass, int32 WeaponAmmo, bool bEquip) {
	AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass);
	AWeapon* ThirdPersonWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass);
	if (Weapon && ThirdPersonWeapon) {
		Ammo.Add(WeaponAmmo);
		
		Weapons.Add(Weapon);
		Weapon->SetOwner(this);
		Weapon->SetFirstPersonWeaponNetMulticast();
		Weapon->AttachToComponent(FirstPersonArms, FAttachmentTransformRules::SnapToTargetIncludingScale, "GripPointSocket");

		ThirdPersonWeapons.Add(ThirdPersonWeapon);
		ThirdPersonWeapon->SetOwner(this);
		ThirdPersonWeapon->SetThirdPersonWeaponNetMulticast();
		ThirdPersonWeapon->AttachToComponent(GetThirdPersonMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "GripPointSocket");
		
		if (bEquip) {
			EquipWeaponServer(Weapons.Num() - 1);
		}
	}
}

void AFPSCharacter::EquipWeaponServer_Implementation(int32 Index) {
	if (Index == EquippedWeaponIndex) return;

	SwitchWeaponServer(Index);
}

void AFPSCharacter::NextWeaponServer_Implementation() {
	if (Weapons.Num() == 0) return;

	int32 NextWeaponIndex = EquippedWeaponIndex + 1;
	if (NextWeaponIndex > Weapons.Num() - 1) {
		NextWeaponIndex = 0;
	}
	if (EquippedWeaponIndex == NextWeaponIndex) return;
	SwitchWeaponServer(NextWeaponIndex);
}

void AFPSCharacter::PreviousWeaponServer_Implementation() {
	if (Weapons.Num() == 0) return;
	
	int32 PreviousWeaponIndex = EquippedWeaponIndex - 1;
	if (PreviousWeaponIndex < 0) {
		PreviousWeaponIndex = Weapons.Num() - 1;
	}
	if (EquippedWeaponIndex == PreviousWeaponIndex) return;
	SwitchWeaponServer(PreviousWeaponIndex);
}

void AFPSCharacter::SwitchWeaponServer_Implementation(int32 Index) {
	if (Weapons.IsValidIndex(Index)) {
		SwitchWeaponNetMulticast(Index);
		EquippedWeaponIndex = Index;
		PlaySoundAtLocationNetMulticast(WeaponSwitchSound);
	}
}

void AFPSCharacter::SwitchCharacter(int32 CharacterListIndex) {
	SwitchCharacterServer(CharacterListIndex);
}

void AFPSCharacter::SwitchCharacterServer_Implementation(int32 CharacterListIndex) {
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController) {
		AFPSPlayerState* FPSPlayerState = PlayerController->GetPlayerState<AFPSPlayerState>();
		if (FPSPlayerState) {
			FPSPlayerState->SetCharacterInfoIndex(CharacterListIndex);
		}
	}
	SwitchCharacterClient(CharacterListIndex);
	SwitchCharacterNetMulticast(CharacterListIndex);
}

void AFPSCharacter::SwitchCharacterClient_Implementation(int32 CharacterListIndex) {
	AFPSGameState* FPSGameState = GetWorld()->GetGameState<AFPSGameState>();
	if (FPSGameState) {
		FirstPersonArms->SetSkeletalMesh(FPSGameState->GetCharacterList()[CharacterListIndex].FirstPersonArms);
		FirstPersonBody->SetSkeletalMesh(FPSGameState->GetCharacterList()[CharacterListIndex].FirstPersonBody);
	}
}

void AFPSCharacter::SwitchCharacterNetMulticast_Implementation(int32 CharacterListIndex) {
	AFPSGameState* FPSGameState = GetWorld()->GetGameState<AFPSGameState>();
    if (FPSGameState) {
		GetThirdPersonMesh()->SetSkeletalMesh(FPSGameState->GetCharacterList()[CharacterListIndex].ThirdPersonMesh);
	}
}

void AFPSCharacter::SwitchWeaponNetMulticast_Implementation(int32 Index) {
	if (Weapons.IsValidIndex(Index)) {
		AWeapon* EquippedWeapon = GetEquippedWeapon();
		if (EquippedWeapon) {
			EquippedWeapon->SetActorHiddenInGame(true);
		}
		GetWeapon(Index)->SetActorHiddenInGame(false);

		AWeapon* ThirdPersonEquippedWeapon = GetThirdPersonEquippedWeapon();
		if (ThirdPersonEquippedWeapon) {
			ThirdPersonEquippedWeapon->SetActorHiddenInGame(true);
			ThirdPersonEquippedWeapon->SetWeaponHiddenShadow(false);
		}
		GetThirdPersonWeapon(Index)->SetActorHiddenInGame(false);
		GetThirdPersonWeapon(Index)->SetWeaponHiddenShadow(true);
	}
}

void AFPSCharacter::AddAmmoServer_Implementation(int32 Index, int32 Amount) {
	AddAmmo(Index, Amount);
}

void AFPSCharacter::DetachEquippedWeaponNetMulticast_Implementation() {
	AWeapon* ThirdPersonEquippedWeapon = GetThirdPersonEquippedWeapon();
	if (ThirdPersonEquippedWeapon) {
		ThirdPersonEquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ThirdPersonEquippedWeapon->RagdollNetMulticast();
	}
}

FVector AFPSCharacter::GetCameraLocation() {
	return CameraComponent->GetComponentLocation();
}

FRotator AFPSCharacter::GetCameraRotation() {
	return CameraComponent->GetComponentRotation();
}

FVector AFPSCharacter::GetForwardVector() {
	return GetControlRotation().Vector();
}

FVector AFPSCharacter::GetRightVector() {
	return UKismetMathLibrary::GetRightVector(GetControlRotation());
}

void AFPSCharacter::AbsorbArmor(float Damage) {
	Damage = (Damage * (1 - ArmorAbsorption)) - FMath::Min(Armor, 0); 
	Armor -= Damage;
}

void AFPSCharacter::Damage(AFPSCharacter* KillerCharacter, FName BoneName, float Damage, float ImpulseDamage, FVector ImpulseDirection) {
	if (!IsAlive()) return;
	// AbsorbArmor(Damage);
	RemoveHealth(Damage);

	if (Health > 0.0f) {
		if (Damage > 0.0f) {
			PlayAnimMontageNetMulticast(DamageAnimMontage);
			PlaySoundAtLocationNetMulticast(DamageSound);
		}
		GetCharacterMovement()->Launch(ImpulseDirection * ImpulseDamage * 0.1f);
	} else {
        Die(KillerCharacter, BoneName, ImpulseDamage, ImpulseDirection);
    }
}

void AFPSCharacter::Die(AFPSCharacter* KillerCharacter, FName BoneName, float ImpulseDamage, FVector ImpulseDirection) {
    if (ImpulseDamage != 0.0f || GetCharacterMovement()->IsFalling()) {
        RagdollNetMulticast(ImpulseDirection * ImpulseDamage, BoneName);
    } else {
        PlayAnimMontageNetMulticast(DeathAnimMontage); // Ragdoll is activated using an AnimNotify inside the montage
    }
    DetachEquippedWeaponNetMulticast();
    GameMode->OnDeath(GetController(), KillerCharacter->GetController(), RespawnTime);
}

void AFPSCharacter::RagdollNetMulticast_Implementation(FVector Impulse, FName BoneName) {
	// DrawDebugLine(GetWorld(), GetActorLocation(), Impulse, FColor::Emerald, false, 5.0f);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	USkeletalMeshComponent* ThirdPersonMesh = GetThirdPersonMesh();
	ThirdPersonMesh->SetCollisionProfileName("Ragdoll");
	ThirdPersonMesh->SetEnableGravity(true);
	ThirdPersonMesh->SetSimulatePhysics(true);
	ThirdPersonMesh->AddImpulse(Impulse, BoneName, true);
}

void AFPSCharacter::RemoveAmmo(int32 Index, int32 Amount) {
	int32 UpdatedAmmo = Ammo[Index];
	UpdatedAmmo -= Amount;
	if (UpdatedAmmo < 0) UpdatedAmmo = 0;
	Ammo[Index] = UpdatedAmmo;
}
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnumTypes.h"
#include "Weapon.h"
#include "MultiplayerFPSGameModeBase.h"

#include "FPSCharacter.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AFPSCharacter : public ACharacter {
	GENERATED_BODY()
private:
	AFPSCharacter();

	UPROPERTY(EditAnywhere) class UCameraComponent* CameraComponent;
	UPROPERTY(EditAnywhere) class USkeletalMeshComponent* FirstPersonArms;
	UPROPERTY(EditAnywhere) class USkeletalMeshComponent* FirstPersonBody;

	UPROPERTY(EditAnywhere) class USoundBase* SpawnSound;
	UPROPERTY(EditAnywhere) class USoundBase* WeaponSwitchSound;
	UPROPERTY(EditAnywhere) class UAnimMontage* DamageAnimMontage;
	UPROPERTY(EditAnywhere) class UAnimMontage* DeathAnimMontage;
	UPROPERTY(EditAnywhere) class USoundBase* DamageSound;
	UPROPERTY(EditAnywhere) class USoundBase* LandSound;

	UPROPERTY(EditAnywhere) class UInputMappingContext* IMC_Player;
	UPROPERTY(EditAnywhere) class UInputAction* IA_Move;
	UPROPERTY(EditAnywhere) class UInputAction* IA_Look;
	UPROPERTY(EditAnywhere) class UInputAction* IA_Jump;
	UPROPERTY(EditAnywhere) class UInputAction* IA_Walk;
	UPROPERTY(EditAnywhere) class UInputAction* IA_Fire;
	UPROPERTY(EditAnywhere) class UInputAction* IA_PreviousWeapon;
	UPROPERTY(EditAnywhere) class UInputAction* IA_NextWeapon;
	UPROPERTY(EditAnywhere) class UInputAction* IA_Scoreboard_UI;
	UPROPERTY(EditAnywhere) class UInputAction* IA_Pause_UI;

	UPROPERTY(EditAnywhere, Replicated) float Health;
	UPROPERTY(EditAnywhere, Replicated) float Armor;
	UPROPERTY(EditAnywhere) float MaxHealth = 100.0f;
	UPROPERTY(EditAnywhere) float MaxArmor = 100.0f;
	UPROPERTY(EditAnywhere) float ArmorAbsorption = 50.0f;

	UPROPERTY(EditAnywhere, Replicated) int32 EquippedWeaponIndex = -1;
	UPROPERTY(EditAnywhere, Replicated) TArray<AWeapon*> Weapons;
	UPROPERTY(EditAnywhere, Replicated) TArray<int32> Ammo;
	UPROPERTY(EditAnywhere, Replicated) TArray<AWeapon*> ThirdPersonWeapons;

	UPROPERTY(ReplicatedUsing=OnRep_CameraRotationPitch) float CameraRotationPitch;
	
	UPROPERTY(EditAnywhere) float RespawnTime = 5.0f;

	AMultiplayerFPSGameModeBase* GameMode;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void FellOutOfWorld(const UDamageType& DmgType) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION() void OnRep_CameraRotationPitch();

	UFUNCTION() void Move(const FInputActionValue& Value);
	UFUNCTION() void Look(const FInputActionValue& Value);
	UFUNCTION() void JumpCharacter(const FInputActionValue& Value);
	UFUNCTION() void ClimbLadder(const FInputActionValue& Value);
	UFUNCTION() void Walk(const FInputActionValue& Value);
	UFUNCTION() void StopWalking(const FInputActionValue& Value);
	UFUNCTION() void ShowScoreboard(const FInputActionValue& Value);
	UFUNCTION() void HideScoreboard(const FInputActionValue& Value);
	UFUNCTION() void TogglePause(const FInputActionValue& Value);
	UFUNCTION() void Fire(const FInputActionValue& Value);
	UFUNCTION() void PreviousWeapon(const FInputActionValue& Value);
	UFUNCTION() void NextWeapon(const FInputActionValue& Value);

	void SetScoreboardVisibility(bool bVisibility);
public:
	UFUNCTION(Server, Reliable) void SetControlRotationPitchServer(float ControlRotationPitch);
	UFUNCTION(Server, Reliable) void AddWeaponServer(TSubclassOf<AWeapon> WeaponClass, int32 WeaponAmmo, bool bEquip = true);
	UFUNCTION(Server, Reliable) void AddAmmoServer(int32 Index, int32 Amount);
	UFUNCTION(Server, Reliable) void EquipWeaponServer(int32 Index);
	UFUNCTION(Server, Reliable) void NextWeaponServer();
	UFUNCTION(Server, Reliable) void PreviousWeaponServer();
	UFUNCTION(Server, Reliable) void SwitchWeaponServer(int32 Index);
	
	UFUNCTION(Client, Unreliable) void PlaySoundAtLocationClient(class USoundBase* Sound);
	UFUNCTION(NetMulticast, Unreliable) void PlaySoundAtLocationNetMulticast(class USoundBase* Sound);
	UFUNCTION(NetMulticast, Unreliable) void PlayAnimMontageNetMulticast(class UAnimMontage* AnimMontage);
	UFUNCTION(NetMulticast, Unreliable) void PlayAnimMontageFirstPersonArmsNetMulticast(UAnimMontage* AnimMontage);
	UFUNCTION(NetMulticast, Reliable) void SwitchWeaponNetMulticast(int32 Index);
	UFUNCTION(NetMulticast, Reliable) void DetachEquippedWeaponNetMulticast();
	UFUNCTION(NetMulticast, Reliable) void RagdollNetMulticast(FVector Impulse = FVector::ZeroVector, FName BoneName = NAME_None);
	
	UFUNCTION(BlueprintCallable) void SwitchCharacter(int32 CharacterListIndex); 
	UFUNCTION(Server, Reliable) void SwitchCharacterServer(int32 CharacterListIndex);
	UFUNCTION(Client, Reliable) void SwitchCharacterClient(int32 CharacterListIndex);
	UFUNCTION(NetMulticast, Reliable) void SwitchCharacterNetMulticast(int32 CharacterListIndex);

	UFUNCTION(BlueprintPure) FVector GetCameraLocation();
	UFUNCTION(BlueprintPure) FRotator GetCameraRotation();
	UFUNCTION(BlueprintPure) FVector GetForwardVector();
	UFUNCTION(BlueprintPure) FVector GetRightVector();
	
	FORCEINLINE class USkeletalMeshComponent* GetThirdPersonMesh() const { return GetMesh(); }

	UFUNCTION(BlueprintPure) FORCEINLINE float GetHealth() { return Health; }
	FORCEINLINE void SetHealth(float InHealth) { Health = InHealth; }
	FORCEINLINE void AddHealth(float Amount) { Health += Amount; }
	FORCEINLINE void RemoveHealth(float Amount) { Health -= Amount; }
	void Damage(AFPSCharacter* KillerCharacter, FName BoneName, float Damage, float ImpulseDamage = 0.0f, FVector ImpulseDirection = FVector::ZeroVector);
	void Die(AFPSCharacter* KillerCharacter, FName BoneName, float ImpulseDamage = 0.0f, FVector ImpulseDirection = FVector::ZeroVector);
	
	UFUNCTION(BlueprintPure) FORCEINLINE float GetArmor() { return Armor; }
	FORCEINLINE void SetArmor(float InArmor) { Armor = InArmor; }
	FORCEINLINE void AddArmor(float Amount) { Armor += Amount; }
	FORCEINLINE void AbsorbArmor(float Damage);

	UFUNCTION(BlueprintPure) FORCEINLINE AWeapon* GetWeapon(int32 Index) { return (Weapons.IsValidIndex(Index)) ? Weapons[Index] : nullptr; }
	
	UFUNCTION(BlueprintPure) FORCEINLINE AWeapon* GetThirdPersonWeapon(int32 Index) { return (ThirdPersonWeapons.IsValidIndex(Index)) ? ThirdPersonWeapons[Index] : nullptr; }

	UFUNCTION(BlueprintPure) FORCEINLINE int32 GetAmmo(int32 Index) { return Ammo[Index]; }
	FORCEINLINE void SetAmmo(int32 Index, int32 InAmmo) { Ammo[Index] = InAmmo; }
	FORCEINLINE void AddAmmo(int32 Index, int32 Amount) { Ammo[Index] += Amount; }
	void RemoveAmmo(int32 Index, int32 Amount);
	
	UFUNCTION(BlueprintPure) FORCEINLINE AWeapon* GetEquippedWeapon() { return (Weapons.IsValidIndex(EquippedWeaponIndex)) ? Weapons[EquippedWeaponIndex] : nullptr; }
	UFUNCTION(BlueprintPure) FORCEINLINE AWeapon* GetThirdPersonEquippedWeapon() { return (ThirdPersonWeapons.IsValidIndex(EquippedWeaponIndex)) ? ThirdPersonWeapons[EquippedWeaponIndex] : nullptr; }
	
	UFUNCTION(BlueprintPure) FORCEINLINE bool HasWeaponEquipped() { return EquippedWeaponIndex != -1; }
	UFUNCTION(BlueprintPure) FORCEINLINE int32 GetEquippedWeaponIndex() { return EquippedWeaponIndex; }
	UFUNCTION(BlueprintPure) FORCEINLINE int32 GetEquippedWeaponAmmo() { return (Ammo.IsValidIndex(EquippedWeaponIndex)) ? Ammo[EquippedWeaponIndex] : 0; }
	FORCEINLINE void ConsumeEquippedWeaponAmmo() { if (Ammo.IsValidIndex(EquippedWeaponIndex)) Ammo[EquippedWeaponIndex]--; }
	UFUNCTION(BlueprintCallable) FORCEINLINE TArray<AWeapon*> GetWeapons() { return Weapons; }
	
	UFUNCTION(BlueprintPure) FORCEINLINE bool IsAlive() { return Health > 0.0f; }
};
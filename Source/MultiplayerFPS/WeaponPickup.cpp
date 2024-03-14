#include "WeaponPickup.h"

void AWeaponPickup::Pickup(AFPSCharacter* CollidingCharacter) {
	if (!WeaponClass) return;
	
	TArray<AWeapon*> Weapons = CollidingCharacter->GetWeapons();
	for (int i = 0; i < Weapons.Num(); i++) {
		if (Weapons[i]->IsA(WeaponClass)) {
			CollidingCharacter->AddAmmoServer(i, WeaponAmmo);
			return;
		}
	}
	
	CollidingCharacter->AddWeaponServer(WeaponClass, WeaponAmmo);
}
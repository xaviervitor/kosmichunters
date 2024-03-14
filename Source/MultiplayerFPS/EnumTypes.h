#pragma once

UENUM()
enum class EWeaponType : int32 {
    Plasma,
    Heavy,
    Launcher,
    MAX
};

UENUM()
enum class EAmmoType : int32 {
    PlasmaBeams,
    LaserBeams,
    LauncherProjectiles,
    MAX
};

UENUM()
enum class EWeaponFireMode: int32 {
    Hitscan,
    Projectile,
    MAX
};
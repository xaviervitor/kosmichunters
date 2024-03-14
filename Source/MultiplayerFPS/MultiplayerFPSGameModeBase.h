#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "MultiplayerFPSGameModeBase.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AMultiplayerFPSGameModeBase : public AGameMode {
	GENERATED_BODY()
	
	int32 KillLimit = 100;

	AMultiplayerFPSGameModeBase();
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
	virtual bool ReadyToEndMatch_Implementation() override;

	void RestartMap() const;

public:
	void OnDeath(AController* DeadPlayerController, AController* KillerPlayerController, float RespawnTime);
	void OnFall(AController* PlayerController);
	UFUNCTION() void DestroyAndRespawn(APawn* PlayerPawn, AController* DeadPlayerController);
};
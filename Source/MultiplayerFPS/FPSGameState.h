#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "FPSPlayerState.h"
#include "CharacterListDataAsset.h"

#include "FPSGameState.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AFPSGameState : public AGameState {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) UCharacterListDataAsset* CharacterListDataAsset;
public:

	int32 KillLimit = 0;

	TArray<AFPSPlayerState*> GetPlayerStatesOrderedByKills();
	UFUNCTION(BlueprintCallable) TArray<FCharacterInfo> GetCharacterList();

	FORCEINLINE void SetKillLimit(int32 InKillLimit) { KillLimit = InKillLimit; }
};

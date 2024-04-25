#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FPSPlayerController.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AFPSPlayerController : public APlayerController {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TSubclassOf<class UPlayerMenu> PlayerMenuClass;
	UPROPERTY(EditAnywhere) bool bIsPaused = false;

	class UPlayerMenu* PlayerMenu;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
public:
	UFUNCTION(BlueprintCallable) void TogglePause();
	void SetScoreboardVisibility(bool bVisibility);
	UFUNCTION(BlueprintCallable) void SetPauseVisibility(bool bVisibility);

	UFUNCTION(Client, Reliable) void ShowScoreboardClient();
	UFUNCTION(Client, Reliable) void NotifyKillClient(const FString& KillerPlayerName, const FString& DeadPlayerName);

	UFUNCTION(BlueprintPure) FORCEINLINE class UPlayerMenu* GetPlayerMenu() { return PlayerMenu; }

	UFUNCTION(BlueprintCallable) FORCEINLINE void SetIsPaused(bool bInIsPaused) { bIsPaused = bInIsPaused; }
	UFUNCTION(BlueprintPure) FORCEINLINE bool GetIsPaused() { return bIsPaused; }
};

#include "FPSPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "PlayerMenu.h"
#include "FPSPlayerState.h"
#include "FPSCharacter.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void AFPSPlayerController::BeginPlay() {
    Super::BeginPlay();
    if(IsLocalController()) {
        if (PlayerMenuClass) {
            PlayerMenu = CreateWidget<UPlayerMenu>(this, PlayerMenuClass);
            if (PlayerMenu) {
                PlayerMenu->AddToViewport(0);
            }
        }
    }
}

void AFPSPlayerController::OnPossess(APawn* InPawn) {
    Super::OnPossess(InPawn);
	AFPSPlayerState* FPSPlayerState = Cast<AFPSPlayerState>(PlayerState);
	if (FPSPlayerState) {
        AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(InPawn);
        if (FPSCharacter) {
            FPSCharacter->SwitchCharacter(FPSPlayerState->GetCharacterInfoIndex());
        }
	}
}

void AFPSPlayerController::SetScoreboardVisibility(bool bVisibility) {
    if (PlayerMenu && !bIsPaused) {
        PlayerMenu->SetScoreboardVisibility(bVisibility);
    }
}

void AFPSPlayerController::TogglePause() {
    bIsPaused = !bIsPaused;
    SetPauseVisibility(bIsPaused);
}

void AFPSPlayerController::SetPauseVisibility(bool bVisibility) {
    if (PlayerMenu) {
        PlayerMenu->SetPauseVisibility(bVisibility);
    }
    bShowMouseCursor = bVisibility;
    bEnableClickEvents = bVisibility;
    bEnableMouseOverEvents = bVisibility;
    if (bVisibility) {
        UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this);
    } else {
        UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
    }
}

void AFPSPlayerController::ShowScoreboardClient_Implementation() {
    SetScoreboardVisibility(true);
}

void AFPSPlayerController::NotifyKillClient_Implementation(const FString& KillerPlayerName, const FString& DeadPlayerName) {
    if (PlayerMenu) {
        PlayerMenu->NotifyPlayerKilled(KillerPlayerName, DeadPlayerName);
    }
}
#include "FPSPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "PlayerMenu.h"
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

void AFPSPlayerController::SetScoreboardVisibility(bool bVisibility) {
    if (PlayerMenu) {
        PlayerMenu->SetScoreboardVisibility(bVisibility);
    }
}

void AFPSPlayerController::TogglePauseVisibility() {
    if (PlayerMenu) {
        PlayerMenu->TogglePauseVisibility();
    }
    bIsPaused = !bIsPaused;
    bShowMouseCursor = bIsPaused;
    bEnableClickEvents = bIsPaused;
    bEnableMouseOverEvents = bIsPaused;
    if (bIsPaused) {
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
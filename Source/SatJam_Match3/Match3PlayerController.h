// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MatchTile.h"
#include "Match3PlayerController.generated.h"

class AMatch3Grid;

UCLASS()
class SATJAM_MATCH3_API AMatch3PlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    // current selected tile
    AMatchTile* SelectedTile = nullptr;

    // convenience cached pointer
    UPROPERTY()
    AMatch3Grid* GridActor = nullptr;

    // input handlers
    void OnLeftClick();

    // helper to find tile under cursor
    AMatchTile* GetTileUnderCursor() const;
};


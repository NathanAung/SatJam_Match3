// Fill out your copyright notice in the Description page of Project Settings.


#include "Match3PlayerController.h"
#include "Match3Grid.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

void AMatch3PlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Find the grid actor in the level
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMatch3Grid::StaticClass(), Found);
    if (Found.Num() > 0)
    {
        GridActor = Cast<AMatch3Grid>(Found[0]);
    }
}

void AMatch3PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction("LeftClick", IE_Pressed, this, &AMatch3PlayerController::OnLeftClick);
    bShowMouseCursor = true;
}

void AMatch3PlayerController::OnLeftClick()
{
    if (!GridActor || GridActor->bInputLocked) return;

    AMatchTile* HitTile = GetTileUnderCursor();
    if (!HitTile) return;

    if (!SelectedTile)
    {
        // pick first tile
        SelectedTile = HitTile;
    }
    else
    {
        // if same tile, deselect
        if (SelectedTile == HitTile)
        {
            SelectedTile = nullptr;
            return;
        }

        // check adjacency
        int dR = FMath::Abs(SelectedTile->Row - HitTile->Row);
        int dC = FMath::Abs(SelectedTile->Col - HitTile->Col);
        if ((dR + dC) == 1)
        {
            GridActor->AttemptSwap(SelectedTile, HitTile);
        }

        SelectedTile = nullptr;
    }
}

AMatchTile* AMatch3PlayerController::GetTileUnderCursor() const
{
    FHitResult Hit;
    bool bHit = GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, Hit);
    if (!bHit) return nullptr;

    return Cast<AMatchTile>(Hit.GetActor());
}

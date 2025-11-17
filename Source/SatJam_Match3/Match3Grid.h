// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MatchTile.h"
#include "Match3Grid.generated.h"

UCLASS()
class SATJAM_MATCH3_API AMatch3Grid : public AActor
{
    GENERATED_BODY()

public:
    AMatch3Grid();

    virtual void BeginPlay() override;

    // grid settings
    UPROPERTY(EditAnywhere, Category = "Grid")
    int32 Rows = 10;

    UPROPERTY(EditAnywhere, Category = "Grid")
    int32 Cols = 6;

    // tile class to spawn (set in Editor)
    UPROPERTY(EditAnywhere, Category = "Grid")
    TSubclassOf<AMatchTile> TileClass;

    // distance between tile centers in world units (X=col spacing, Y=row spacing)
    UPROPERTY(EditAnywhere, Category = "Grid")
    FVector CellSize = FVector(100.f, 100.f, 0.f);

    // world origin (top-left) for grid placement
    UPROPERTY(EditAnywhere, Category = "Grid")
    FVector GridOrigin = FVector::ZeroVector;

    // score and winning
    UPROPERTY(VisibleAnywhere, Category = "Game")
    int32 Score = 0;

    UPROPERTY(EditAnywhere, Category = "Game")
    int32 PointsPerClear = 100;

    UPROPERTY(EditAnywhere, Category = "Game")
    int32 WinScore = 1000;

    // prevent player input during clears
    UPROPERTY(VisibleAnywhere, Category = "Game")
    bool bInputLocked = false;

    float ClearDelay = 0.3f;    // Time before clearing


    // public accessors
    AMatchTile* GetTileAt(int32 Row, int32 Col) const;
    bool IsInside(int32 Row, int32 Col) const;

    // swap two tiles (called by player controller)
    void AttemptSwap(AMatchTile* A, AMatchTile* B);

    // regenerate grid (used on start and if no moves)
    void RegenerateGrid();

protected:
    // internal grid storage (flattened)
    TArray<AMatchTile*> GridArray;

    TArray<AMatchTile*> PendingClearMatches;
    FTimerHandle ClearTimerHandle;


    // helpers
    inline int32 Index(int32 Row, int32 Col) const { return Row * Cols + Col; }

    void FillGridRandomly();
    bool HasAnyMatches() const;
    TArray<AMatchTile*> FindAllMatches() const;
    void ClearMatches(const TArray<AMatchTile*>& Matches);
    void ApplyGravityAndRefill();
    bool HasPossibleMove() const;

    void StartClear(const TArray<AMatchTile*>& Matches);
    void PerformClear();

    // utility
    void DestroyAllTiles();
    void SpawnTileAt(int32 Row, int32 Col, ETileColor Color);
};

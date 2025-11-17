// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MatchTile.generated.h"

UENUM(BlueprintType)
enum class ETileColor : uint8
{
    Red,
    Blue,
    Green,
    Yellow
};

UCLASS()
class SATJAM_MATCH3_API AMatchTile : public AActor
{
    GENERATED_BODY()

public:
    AMatchTile();

    UPROPERTY(VisibleAnywhere, Category = "Tile")
    UStaticMeshComponent* TileMesh;

    // grid coordinates
    UPROPERTY(VisibleAnywhere, Category = "Tile")
    int32 Row;

    UPROPERTY(VisibleAnywhere, Category = "Tile")
    int32 Col;

    // tile color
    UPROPERTY(VisibleAnywhere, Category = "Tile")
    ETileColor Color;

    UPROPERTY(EditAnywhere, Category = "Tile")
    UMaterialInterface* RedMaterial;

    UPROPERTY(EditAnywhere, Category = "Tile")
    UMaterialInterface* BlueMaterial;

    UPROPERTY(EditAnywhere, Category = "Tile")
    UMaterialInterface* GreenMaterial;

    UPROPERTY(EditAnywhere, Category = "Tile")
    UMaterialInterface* YellowMaterial;

    // det the tile's logical grid position and move it to world location
    void SetGridPosition(int32 NewRow, int32 NewCol, const FVector& CellSize, const FVector& GridOrigin);

    // change color and update visual
    void SetColor(ETileColor NewColor);

    // returns world location for given row/col
    static FVector GetWorldLocationForGrid(int32 InRow, int32 InCol, const FVector& CellSize, const FVector& GridOrigin);
};


// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchTile.h"
#include "Components/StaticMeshComponent.h"


AMatchTile::AMatchTile()
{
    PrimaryActorTick.bCanEverTick = false;

    TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
    RootComponent = TileMesh;

    // default settings
    TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TileMesh->SetGenerateOverlapEvents(false);

    Row = Col = 0;
    Color = ETileColor::Red;
}


// position in grid 
void AMatchTile::SetGridPosition(int32 NewRow, int32 NewCol, const FVector& CellSize, const FVector& GridOrigin)
{
    Row = NewRow;
    Col = NewCol;
    FVector World = GetWorldLocationForGrid(Row, Col, CellSize, GridOrigin);
    SetActorLocation(World);
}


// for color switching
void AMatchTile::SetColor(ETileColor NewColor)
{
    Color = NewColor;

    // set material
    switch (Color)
    {
    case ETileColor::Red:
        TileMesh->SetMaterial(0, RedMaterial);
        break;

    case ETileColor::Blue:
        TileMesh->SetMaterial(0, BlueMaterial);
        break;

    case ETileColor::Green:
        TileMesh->SetMaterial(0, GreenMaterial);
        break;

    case ETileColor::Yellow:
        TileMesh->SetMaterial(0, YellowMaterial);
        break;
    }
}


// location in world
FVector AMatchTile::GetWorldLocationForGrid(int32 InRow, int32 InCol, const FVector& CellSize, const FVector& GridOrigin)
{
    // using X = Col * CellSize.X, Y = Row * CellSize.Y, Z from GridOrigin.Z
    return GridOrigin + FVector(InCol * CellSize.X, InRow * CellSize.Y, 0.f);
}



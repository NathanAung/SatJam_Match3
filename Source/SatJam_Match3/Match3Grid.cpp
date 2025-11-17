// Fill out your copyright notice in the Description page of Project Settings.


#include "Match3Grid.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

AMatch3Grid::AMatch3Grid()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMatch3Grid::BeginPlay()
{
    Super::BeginPlay();

    // initialize array
    GridArray.SetNumZeroed(Rows * Cols);
    RegenerateGrid();
}


void AMatch3Grid::RegenerateGrid()
{
    // destroy any existing tiles
    DestroyAllTiles();

    // try generating until the rules are satisfied
    // - no initial 3+ matches
    // - at least one possible move
    // simple loop with safety limit
    const int32 MaxAttempts = 50;
    int32 Attempt = 0;

    do
    {
        DestroyAllTiles();
        FillGridRandomly();
        Attempt++;
        if (Attempt >= MaxAttempts)
        {
            UE_LOG(LogTemp, Warning, TEXT("RegenerateGrid: Max attempts reached; accepting current grid."));
            break;
        }
    } while (HasAnyMatches() || !HasPossibleMove());

    Score = 0;
    bInputLocked = false;
}


// make sure that there are no matches
void AMatch3Grid::FillGridRandomly()
{
    GridArray.Init(nullptr, Rows * Cols);

    for (int r = 0; r < Rows; ++r)
    {
        for (int c = 0; c < Cols; ++c)
        {
            ETileColor Color;

            while (true)
            {
                Color = static_cast<ETileColor>(FMath::RandRange(0, 3));

                // Check horizontal
                bool badHorizontal =
                    c >= 2 &&
                    GetTileAt(r, c - 1) &&
                    GetTileAt(r, c - 2) &&
                    GetTileAt(r, c - 1)->Color == Color &&
                    GetTileAt(r, c - 2)->Color == Color;

                // Check vertical
                bool badVertical =
                    r >= 2 &&
                    GetTileAt(r - 1, c) &&
                    GetTileAt(r - 2, c) &&
                    GetTileAt(r - 1, c)->Color == Color &&
                    GetTileAt(r - 2, c)->Color == Color;

                if (!badHorizontal && !badVertical)
                    break;
            }

            SpawnTileAt(r, c, Color);
        }
    }
}


// individual tile
void AMatch3Grid::SpawnTileAt(int32 Row, int32 Col, ETileColor Color)
{
    if (!TileClass) return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    FVector Loc = AMatchTile::GetWorldLocationForGrid(Row, Col, CellSize, GridOrigin);
    AMatchTile* Tile = GetWorld()->SpawnActor<AMatchTile>(TileClass, Loc, FRotator::ZeroRotator, Params);
    if (!Tile) return;

    Tile->SetGridPosition(Row, Col, CellSize, GridOrigin);
    Tile->SetColor(Color);

    GridArray[Index(Row, Col)] = Tile;
}


AMatchTile* AMatch3Grid::GetTileAt(int32 Row, int32 Col) const
{
    if (!IsInside(Row, Col)) return nullptr;
    return GridArray.IsValidIndex(Index(Row, Col)) ? GridArray[Index(Row, Col)] : nullptr;
}


bool AMatch3Grid::IsInside(int32 Row, int32 Col) const
{
    return Row >= 0 && Row < Rows && Col >= 0 && Col < Cols;
}


// find all matches (3+ horizontal or vertical) and return unique tile list
TArray<AMatchTile*> AMatch3Grid::FindAllMatches() const
{
    TArray<AMatchTile*> Matches;
    Matches.Reserve(Rows * Cols / 4);

    // Horizontal
    for (int r = 0; r < Rows; ++r)
    {
        for (int c = 0; c <= Cols - 3; ++c)
        {
            AMatchTile* A = GetTileAt(r, c);
            AMatchTile* B = GetTileAt(r, c + 1);
            AMatchTile* C = GetTileAt(r, c + 2);
            if (!A || !B || !C) continue;
            if (A->Color == B->Color && B->Color == C->Color)
            {
                // extend run
                int runEnd = c + 2;
                while (runEnd + 1 < Cols && GetTileAt(r, runEnd + 1) && GetTileAt(r, runEnd + 1)->Color == A->Color)
                {
                    runEnd++;
                }
                for (int k = c; k <= runEnd; ++k)
                {
                    Matches.AddUnique(GetTileAt(r, k));
                }
                c = runEnd;
            }
        }
    }

    // Vertical
    for (int c = 0; c < Cols; ++c)
    {
        for (int r = 0; r <= Rows - 3; ++r)
        {
            AMatchTile* A = GetTileAt(r, c);
            AMatchTile* B = GetTileAt(r + 1, c);
            AMatchTile* C = GetTileAt(r + 2, c);
            if (!A || !B || !C) continue;
            if (A->Color == B->Color && B->Color == C->Color)
            {
                int runEnd = r + 2;
                while (runEnd + 1 < Rows && GetTileAt(runEnd + 1, c) && GetTileAt(runEnd + 1, c)->Color == A->Color)
                {
                    runEnd++;
                }
                for (int k = r; k <= runEnd; ++k)
                {
                    Matches.AddUnique(GetTileAt(k, c));
                }
                r = runEnd;
            }
        }
    }

    return Matches;
}


bool AMatch3Grid::HasAnyMatches() const
{
    TArray<AMatchTile*> Found = FindAllMatches();
    return Found.Num() > 0;
}


// try swapping two tiles, actual swap only kept if it results in at least one match
void AMatch3Grid::AttemptSwap(AMatchTile* A, AMatchTile* B)
{
    if (!A || !B) return;
    if (bInputLocked) return;

    // ensure A and B are adjacent
    int dR = FMath::Abs(A->Row - B->Row);
    int dC = FMath::Abs(A->Col - B->Col);
    if ((dR + dC) != 1) return;

    // swap in array
    int rA = A->Row, cA = A->Col;
    int rB = B->Row, cB = B->Col;

    GridArray[Index(rA, cA)] = B;
    GridArray[Index(rB, cB)] = A;

    // update positions immediately (no anim)
    A->SetGridPosition(rB, cB, CellSize, GridOrigin);
    B->SetGridPosition(rA, cA, CellSize, GridOrigin);

    // check for matches
    TArray<AMatchTile*> Matches = FindAllMatches();
    if (Matches.Num() == 0)
    {
        // swap back
        GridArray[Index(rA, cA)] = A;
        GridArray[Index(rB, cB)] = B;
        A->SetGridPosition(rA, cA, CellSize, GridOrigin);
        B->SetGridPosition(rB, cB, CellSize, GridOrigin);
        return;
    }

    // have matches => resolve them
    bInputLocked = true;
    ClearMatches(Matches);
}


void AMatch3Grid::ClearMatches(const TArray<AMatchTile*>& Matches)
{
    if (Matches.Num() == 0)
    {
        bInputLocked = false;
        return;
    }

    // scoring: award 100 points per clear, count one clear per tile-grouping:
    int32 Award = PointsPerClear * FMath::Max(1, Matches.Num() / 3);
    Score += Award;
    UE_LOG(LogTemp, Log, TEXT("Cleared %d tiles. Score=%d"), Matches.Num(), Score);

    // remove tiles
    for (AMatchTile* Tile : Matches)
    {
        if (!Tile) continue;
        int r = Tile->Row;
        int c = Tile->Col;
        GridArray[Index(r, c)] = nullptr;
        Tile->Destroy();
    }

    // gravity and refill
    ApplyGravityAndRefill();

    // chains: find new matches and recursively clear
    TArray<AMatchTile*> NewMatches = FindAllMatches();
    if (NewMatches.Num() > 0)
    {
        ClearMatches(NewMatches);
    }
    else
    {
        bInputLocked = false;

        // after resolution, if no possible moves, regenerate
        if (!HasPossibleMove())
        {
            UE_LOG(LogTemp, Log, TEXT("No possible moves — regenerating grid"));
            RegenerateGrid();
            return;
        }

        // win check
        if (Score >= WinScore)
        {
            UE_LOG(LogTemp, Log, TEXT("YOU WIN! Score=%d"), Score);
            // UI HERE
        }
    }
}


void AMatch3Grid::ApplyGravityAndRefill()
{
    // shift tiles down column by column
    for (int c = 0; c < Cols; ++c)
    {
        int writeRow = Rows - 1;
        for (int r = Rows - 1; r >= 0; --r)
        {
            AMatchTile* Tile = GetTileAt(r, c);
            if (Tile)
            {
                if (writeRow != r)
                {
                    GridArray[Index(writeRow, c)] = Tile;
                    Tile->SetGridPosition(writeRow, c, CellSize, GridOrigin);
                    GridArray[Index(r, c)] = nullptr;
                }
                writeRow--;
            }
        }

        // fill remaining above
        for (int r = writeRow; r >= 0; --r)
        {
            ETileColor Color = static_cast<ETileColor>(FMath::RandRange(0, 3));
            SpawnTileAt(r, c, Color);
        }
    }
}


// detect if any single adjacent swap would create a match
bool AMatch3Grid::HasPossibleMove() const
{
    // for each pair of adjacent tiles, simulate swap and test for any matches
    auto SimSwapCreatesMatch = [&](int r1, int c1, int r2, int c2) -> bool
        {
            if (!IsInside(r1, c1) || !IsInside(r2, c2)) return false;
            AMatchTile* T1 = GetTileAt(r1, c1);
            AMatchTile* T2 = GetTileAt(r2, c2);
            if (!T1 || !T2) return false;

            // swap colors only (fast)
            ETileColor Col1 = T1->Color;
            ETileColor Col2 = T2->Color;

            // perform color swap in temp and check nearby matches
            auto CheckMatchAt = [&](int r, int c) -> bool
                {
                    // horizontal
                    int count = 1;
                    // left
                    for (int cc = c - 1; cc >= 0; --cc)
                    {
                        AMatchTile* S = (r == r1 && cc == c1) ? ((r2 == r1 && cc == c2) ? nullptr : GetTileAt(r, cc)) : GetTileAt(r, cc);
                    }

                    auto GetColorWithSwap = [&](int rr, int cc)->ETileColor
                        {
                            if (rr == r1 && cc == c1) return Col2;
                            if (rr == r2 && cc == c2) return Col1;
                            AMatchTile* T = GetTileAt(rr, cc);
                            return T ? T->Color : ETileColor::Red; // default
                        };

                    // horizontal count
                    ETileColor center = GetColorWithSwap(r, c);
                    if (center == ETileColor::Red && !GetTileAt(r, c)) return false;
                    int left = 0;
                    for (int cc = c - 1; cc >= 0; --cc) {
                        if (GetColorWithSwap(r, cc) == center) left++; else break;
                    }
                    int right = 0;
                    for (int cc = c + 1; cc < Cols; ++cc) {
                        if (GetColorWithSwap(r, cc) == center) right++; else break;
                    }
                    if (left + 1 + right >= 3) return true;

                    // vertical
                    int up = 0;
                    for (int rr = r - 1; rr >= 0; --rr) {
                        if (GetColorWithSwap(rr, c) == center) up++; else break;
                    }
                    int down = 0;
                    for (int rr = r + 1; rr < Rows; ++rr) {
                        if (GetColorWithSwap(rr, c) == center) down++; else break;
                    }
                    if (up + 1 + down >= 3) return true;

                    return false;
                };

            // check tiles near both positions
            for (int dr = -2; dr <= 2; ++dr)
            {
                for (int dc = -2; dc <= 2; ++dc)
                {
                    int rr = r1 + dr;
                    int cc = c1 + dc;
                    if (IsInside(rr, cc) && CheckMatchAt(rr, cc)) return true;
                    rr = r2 + dr;
                    cc = c2 + dc;
                    if (IsInside(rr, cc) && CheckMatchAt(rr, cc)) return true;
                }
            }
            return false;
        };

    for (int r = 0; r < Rows; ++r)
    {
        for (int c = 0; c < Cols; ++c)
        {
            // right
            if (c + 1 < Cols)
            {
                if (SimSwapCreatesMatch(r, c, r, c + 1)) return true;
            }
            // down
            if (r + 1 < Rows)
            {
                if (SimSwapCreatesMatch(r, c, r + 1, c)) return true;
            }
        }
    }

    return false;
}


void AMatch3Grid::DestroyAllTiles()
{
    for (int i = 0; i < GridArray.Num(); ++i)
    {
        AMatchTile* T = GridArray[i];
        if (T)
        {
            T->Destroy();
            GridArray[i] = nullptr;
        }
    }
}



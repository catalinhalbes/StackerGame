#include "GameBoard.h"

#include <iostream>

GameBoard::GameBoard(const std::unordered_map<MinoType, std::array<Tetromino, 4>>& rotationOffsets, const std::unordered_map<MinoType, std::vector<std::vector<std::vector<glm::ivec2>>>>& kickTable)
{
    this->RotationOffsets = rotationOffsets;
    this->KickTable = kickTable;
}

void GameBoard::Load()
{
    ClearBoard();
    TetrominoQueue.clear();
    IsOver = false;
    IsPaused = true;
    HoldUsed = false;
    LinesCleared = 0;
    PiecesPlaced = 0;
    Combo = 0;

    /*
    SetMatrix({
        { GARBAGE, EMPTY,   GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, EMPTY,   GARBAGE },
        { EMPTY,   GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, EMPTY   },
        { EMPTY,   EMPTY,   GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, EMPTY,   EMPTY   },
        { EMPTY,   GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, GARBAGE, EMPTY   },
        { EMPTY,   EMPTY,   EMPTY,   GARBAGE, GARBAGE, GARBAGE, GARBAGE, EMPTY,  EMPTY,   EMPTY   },
        { GARBAGE, EMPTY,   EMPTY,   GARBAGE, GARBAGE, GARBAGE, GARBAGE, EMPTY,   EMPTY,   GARBAGE },
        //{ GARBAGE, EMPTY,   EMPTY,   GARBAGE, GARBAGE, GARBAGE, GARBAGE, EMPTY,   EMPTY,   GARBAGE },
    });

    TetrominoQueue.push_back(MinoType::BLOCK_T);
    TetrominoQueue.push_back(MinoType::BLOCK_T);
    TetrominoQueue.push_back(MinoType::BLOCK_T);
    TetrominoQueue.push_back(MinoType::BLOCK_T);
    TetrominoQueue.push_back(MinoType::BLOCK_O);
    TetrominoQueue.push_back(MinoType::BLOCK_O);
    */

    std::random_device dev;
    BagSeed = dev();
    BagRNG.seed(BagSeed);

    PopulateQueue();

    HoldPiece = MinoType::EMPTY;
    NextPiece();

/*
    for (int i = 0; i < 40; i++)
    {
        MinoType t = (MinoType)((i % 7) + 1);
        for (int j = 0; j < 10; j++)
        {
            if (j < 3 || j > 6)
            {
                Matrix[i][j] = t;
            }
        }
    }
    Matrix[0][3] = MinoType::GARBAGE;
    Matrix[1][3] = MinoType::GARBAGE;
    Matrix[0][4] = MinoType::GARBAGE;
*/

    GhostPosition = SoftDropPosition();
    StartTime = StopTime = std::chrono::high_resolution_clock::now();
}

void GameBoard::ClearBoard()
{
    // clear game objects
    for (int i = 0; i < 40; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            Matrix[i][j] = MinoType::EMPTY;
        }
    }
}

void GameBoard::SetMatrix(const std::vector<std::vector<MinoType>>& matrix)
{
    ClearBoard();
    for (size_t row = 0; row < matrix.size(); row++)
    {
        for (size_t col = 0; col < matrix[row].size(); col++)
        {
            Matrix[row][col] = matrix[row][col];
        }
    }
}

void GameBoard::ExecuteMoves(const std::list<MoveType>& moves)
{
    if (IsOver || IsPaused) return;

    glm::ivec2 newPos;
    for (MoveType move: moves)
    {
        newPos = CurrentPosition;

        switch (move)
        {
            case MOVE_LEFT:
                newPos.y = newPos.y - 1;
                if (CurrentPieceCanMoveAt(newPos, CurrentRotation))
                    CurrentPosition = newPos;
                break;

            case MOVE_RIGHT:
                newPos.y = newPos.y + 1;
                if (CurrentPieceCanMoveAt(newPos, CurrentRotation))
                    CurrentPosition = newPos;
                break;

            case DAS_LEFT:
                while(CurrentPieceCanMoveAt(newPos, CurrentRotation))
                    newPos.y = newPos.y - 1;
                newPos.y = newPos.y + 1;
                CurrentPosition = newPos;
                break;

            case DAS_RIGHT:
                while(CurrentPieceCanMoveAt(newPos, CurrentRotation))
                    newPos.y = newPos.y + 1;
                newPos.y = newPos.y - 1;
                CurrentPosition = newPos;
                break;

            case ROTATE_CLOCKWISE:
            case ROTATE_ANTICLOCKWISE:
            case ROTATE_180:
                RotateWithKick(move);
                break;

            case HARDDROP: { 
                // declaring variables inside case makes them visible to the other cases, but the initializer is in the case they were declared
                // so we put the code inside another block to prevent that

                // add piece to boad at the location of ghost piece/ soft drop location
                const auto &offsets = RotationOffsets.at(CurrentPiece)[CurrentRotation].PieceOffsets;
                glm::ivec2 pos = SoftDropPosition();

                Matrix[pos.x + offsets[0].x][pos.y + offsets[0].y] = CurrentPiece;
                Matrix[pos.x + offsets[1].x][pos.y + offsets[1].y] = CurrentPiece;
                Matrix[pos.x + offsets[2].x][pos.y + offsets[2].y] = CurrentPiece;
                Matrix[pos.x + offsets[3].x][pos.y + offsets[3].y] = CurrentPiece;

                int cleared = ClearLines();
                LinesCleared = LinesCleared + cleared;

                NextPiece();
                if (!CurrentPieceCanMoveAt(CurrentPosition, CurrentRotation))
                {
                    Stop();
                    IsOver = true;
                }

                HoldUsed = false;
                PiecesPlaced = PiecesPlaced + 1;

                if (cleared)
                    Combo = Combo + 1;
                else
                    Combo = 0;


                break;
            }

            case SOFTDROP:
                CurrentPosition = SoftDropPosition();
                break;

            case HOLD:
                if (HoldUsed) break;
                HoldUsed = true;

                if (HoldPiece == MinoType::EMPTY)
                {
                    HoldPiece = CurrentPiece;
                    NextPiece();
                }
                else
                {
                    MinoType aux = HoldPiece;
                    HoldPiece = CurrentPiece;
                    CurrentPiece = aux;
                }
                break;

            // debug cases
            case MOVE_UP:
                newPos.x = newPos.x + 1;
                if (CurrentPieceCanMoveAt(newPos, CurrentRotation))
                    CurrentPosition = newPos;
                break;

            case MOVE_DOWN:
                newPos.x = newPos.x - 1;
                if (CurrentPieceCanMoveAt(newPos, CurrentRotation))
                    CurrentPosition = newPos;
                break;

            case NO_MOVE:
                break;
        }
    }
    GhostPosition = SoftDropPosition();
}

std::array<MinoType, 7> GameBoard::GetNextBag()
{
    std::array<MinoType, 7> bag = SEVEN_PIECE_BAG;
    std::ranges::shuffle(bag, BagRNG);
    return bag;
}

void GameBoard::PopulateQueue()
{
    while (TetrominoQueue.size() <= PREVIEW_NUMBER)
    {
        auto bag = GetNextBag();
        TetrominoQueue.insert(TetrominoQueue.end(), bag.begin(), bag.end());
    }
}

glm::ivec2 GameBoard::SoftDropPosition()
{
    glm::ivec2 pos = CurrentPosition;
    while (CurrentPieceCanMoveAt(pos, CurrentRotation))
        pos.x = pos.x - 1;
    pos.x = pos.x + 1;
    return pos;
}

bool GameBoard::CurrentPieceCanMoveAt(glm::ivec2 position, int rotation)
{
    const auto &offsets = RotationOffsets.at(CurrentPiece)[rotation].PieceOffsets;
    bool res =
            position.x + offsets[0].x >= 0 && position.y + offsets[0].y >= 0 && position.y + offsets[0].y < 10 &&
            position.x + offsets[1].x >= 0 && position.y + offsets[1].y >= 0 && position.y + offsets[1].y < 10 &&
            position.x + offsets[2].x >= 0 && position.y + offsets[2].y >= 0 && position.y + offsets[2].y < 10 &&
            position.x + offsets[3].x >= 0 && position.y + offsets[3].y >= 0 && position.y + offsets[3].y < 10 &&
            !(
                Matrix[position.x + offsets[0].x][position.y + offsets[0].y] ||
                Matrix[position.x + offsets[1].x][position.y + offsets[1].y] ||
                Matrix[position.x + offsets[2].x][position.y + offsets[2].y] ||
                Matrix[position.x + offsets[3].x][position.y + offsets[3].y]
            );
    return res;
}

void GameBoard::NextPiece()
{
    CurrentPiece = TetrominoQueue.front();
    TetrominoQueue.pop_front();
    PopulateQueue();
    CurrentPosition = glm::ivec2(21, 4);
    CurrentRotation = 0;
}

unsigned int GameBoard::ClearLines()
{
    unsigned int cleared = 0;
    bool rowFull;
    for (int row = 0; row < 40; row++)
    {
        // check if current row is full (aka doesn't contain air blocks or solid garbage)
        rowFull = true;
        for (int col = 0; col < 10; col++)
        {
            if (Matrix[row][col] == MinoType::EMPTY || Matrix[row][col] == MinoType::SOLID_GARBAGE)
            {
                rowFull = false;
                break;
            }
        }

        // if there are lines "to be cleared" shift all the rows above
        if (rowFull)
        {
            cleared = cleared + 1;
            for (int above = row; above < 39; above++)
            {
                for(int col = 0; col < 10; col++)
                    Matrix[above][col] = Matrix[above + 1][col];
            }
            row = row - 1;
        }
    }

    // clear the top rows so they don't get cloned
    for (int row = 39 - cleared; row < 40; row++)
        for (int col = 0; col < 10; col++)
            Matrix[row][col] = MinoType::EMPTY;

    return cleared;
}

bool GameBoard::RotateWithKick(MoveType rot)
{
    int newRot = CurrentRotation;
    switch(rot)
    {
        case ROTATE_CLOCKWISE:
            newRot = (CurrentRotation + 1) % 4;
            break;

        case ROTATE_ANTICLOCKWISE:
            newRot = (CurrentRotation + 3) % 4; // current rotation - 1 + 4
            break;

        case ROTATE_180:
            newRot = (CurrentRotation + 2) % 4;
            break;

        default:
            return false;
    }

    for(glm::ivec2 offset: KickTable.at(CurrentPiece)[CurrentRotation][newRot]) {
        if (CurrentPieceCanMoveAt(CurrentPosition + offset, newRot)) {
            CurrentRotation = newRot;
            CurrentPosition = CurrentPosition + offset;
            return true;
        }
    }

    return false;
}

std::chrono::duration<double> GameBoard::GetElapsedTime()
{
    if (IsOver || IsPaused)
        return StopTime - StartTime;
    return std::chrono::high_resolution_clock::now() - StartTime;
}

void GameBoard::Start()
{
    IsPaused = false;
    StartTime = std::chrono::high_resolution_clock::now();
}


void GameBoard::Stop()
{
    IsPaused = true;
    StopTime = std::chrono::high_resolution_clock::now();
}

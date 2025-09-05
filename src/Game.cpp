#include "Game.h"

#include <iostream>

const glm::vec2 Game::BOARD_SIZE = glm::vec2(300.0f, 600.0f);
const glm::vec2 Game::BOARD_POS  = glm::vec2(230.0f, 100.0f);
const glm::vec2 Game::BOARD_BACK_POS = glm::vec2(50.0f, 100.0f);
const glm::vec2 Game::BOARD_BACK_SIZE = glm::vec2(660.0f, 780.0f);
const float Game::GHOST_OPACITY  = 0.25f;

Game::Game(unsigned int width, unsigned int height, const GameSettings& settings)
:   Settings(settings),
    State(GAME_ACTIVE), 
    Keys(), 
    KeysProcessed(), 
    DASHeldTime(0.0f), 
    ARRHeldTime(-1.0f),
    SoftDropHeldTime(0.0f),
    PreviousDASDirection(MoveType::NO_MOVE), 
    Width(width), 
    Height(height)
{

}

Game::~Game()
{
    delete SpriteRender;
    delete TextRender;
    delete Board;
}

void Game::Init()
{
    // load shaders
    ResourceManager::LoadShader("shaders/sprite.vert", "shaders/sprite.frag", nullptr, "sprite");

    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);

    // set render-specific controls
    SpriteRender = new SpriteRenderer(ResourceManager::GetShader("sprite"));

    // configure text renderer
    TextRender = new TextRenderer(this->Width, this->Height);
    TextRender->Load("fonts/freemono/FreeMonospacedBold-m5VP.ttf", BOARD_SIZE.x / 10);

    // load textures
    ResourceManager::LoadTexture("textures/background.png", false, "background");
    ResourceManager::LoadTexture("textures/block.png", false, "block");
    ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/board2.png", true, "back_board");
    ResourceManager::LoadTexture("textures/x.png", true, "spawn_preview");

    // prepare board
    Board = new GameBoard(SRS_TETROMINO_ROTATIONS, SRS_PLUS_KICK_TABLE);
    BoardSize = BOARD_SIZE;
    BoardPosition = BOARD_POS;
    MinoSize = glm::vec2(BoardSize.x / 10.0f, BoardSize.y / 20.0f);

    BoardStartPosition = glm::vec2(BoardPosition.x, BoardPosition.y + BoardSize.y - MinoSize.y);
    PreviewStartPosition = glm::vec2(BoardPosition.x + BoardSize.x + MinoSize.x * 2.0f, BoardPosition.y + MinoSize.y * 2.0f);
    HoldPosition = glm::vec2(BoardPosition.x - MinoSize.x * 4.0f, BoardPosition.y + MinoSize.y * 2.0f);
    SpawnPosition = glm::vec2(BoardPosition.x + MinoSize.x * 4.0f, BoardPosition.y - MinoSize.y * 2.0f);

    StatsStartPosition = BoardStartPosition + MinoSize * glm::vec2(4.0f, 1.05f);
    StatsSpacing = glm::vec2(0.0f, MinoSize.y * 1.05f);

    Board->Load();

    // generate shape parts from block texture by setting color
    MinoColors[MinoType::BLOCK_L] = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
    MinoColors[MinoType::BLOCK_J] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    MinoColors[MinoType::BLOCK_I] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    MinoColors[MinoType::BLOCK_O] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    MinoColors[MinoType::BLOCK_T] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    MinoColors[MinoType::BLOCK_S] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    MinoColors[MinoType::BLOCK_Z] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    // ghost variants have the same color, but lower opacity
    MinoColors[MinoType::GHOST_L] = MinoColors[MinoType::BLOCK_L]; MinoColors[MinoType::GHOST_L].a = GHOST_OPACITY;
    MinoColors[MinoType::GHOST_J] = MinoColors[MinoType::BLOCK_J]; MinoColors[MinoType::GHOST_J].a = GHOST_OPACITY;
    MinoColors[MinoType::GHOST_I] = MinoColors[MinoType::BLOCK_I]; MinoColors[MinoType::GHOST_I].a = GHOST_OPACITY;
    MinoColors[MinoType::GHOST_O] = MinoColors[MinoType::BLOCK_O]; MinoColors[MinoType::GHOST_O].a = GHOST_OPACITY;
    MinoColors[MinoType::GHOST_T] = MinoColors[MinoType::BLOCK_T]; MinoColors[MinoType::GHOST_T].a = GHOST_OPACITY;
    MinoColors[MinoType::GHOST_S] = MinoColors[MinoType::BLOCK_S]; MinoColors[MinoType::GHOST_S].a = GHOST_OPACITY;
    MinoColors[MinoType::GHOST_Z] = MinoColors[MinoType::BLOCK_Z]; MinoColors[MinoType::GHOST_Z].a = GHOST_OPACITY;

    // garbage and other variants
    MinoColors[MinoType::GARBAGE] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    MinoColors[MinoType::SOLID_GARBAGE] = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    MinoColors[MinoType::SPAWN_PREVIEW] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    // set mino textures
    MinoTexture[MinoType::BLOCK_L] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::BLOCK_J] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::BLOCK_I] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::BLOCK_O] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::BLOCK_T] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::BLOCK_S] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::BLOCK_Z] = ResourceManager::GetTexture("block");

    MinoTexture[MinoType::GHOST_L] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::GHOST_J] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::GHOST_I] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::GHOST_O] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::GHOST_T] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::GHOST_S] = ResourceManager::GetTexture("block");
    MinoTexture[MinoType::GHOST_Z] = ResourceManager::GetTexture("block");

    MinoTexture[MinoType::GARBAGE] = ResourceManager::GetTexture("block_solid");
    MinoTexture[MinoType::SOLID_GARBAGE] = ResourceManager::GetTexture("block_solid");
    MinoTexture[MinoType::SPAWN_PREVIEW] = ResourceManager::GetTexture("spawn_preview");
}

void Game::Update(float dt)
{

}

void Game::ProcessInput(float dt)
{
    if (State == GAME_ACTIVE)
    {
        std::list<MoveType> movelist;

        MoveType das_direction = MoveType::NO_MOVE;
        MoveType move_direction = MoveType::NO_MOVE;

        if (Keys[Settings.Restart] && !KeysProcessed[Settings.Restart]) {
            Board->Load();
            KeysProcessed[Settings.Restart] = true;
        }

        if (Board->IsOver) {
            return;
        }

        // Process DAS
        if (KeysProcessed[Settings.MoveLeft] || KeysProcessed[Settings.MoveRight]) 
        {
            DASHeldTime += dt;

            if (KeysProcessed[Settings.MoveLeft] && KeysProcessed[Settings.MoveRight]) 
            {
                // if pressed at the same time, the last pressed takes precedence
                if (PreviousDASDirection == MoveType::DAS_LEFT) 
                {
                    das_direction = MoveType::DAS_RIGHT;
                    move_direction = MoveType::MOVE_RIGHT;
                }
                else 
                {
                    das_direction = MoveType::DAS_LEFT;
                    move_direction = MoveType::MOVE_LEFT;
                }

                if (Settings.ResetDASOnDirectionChange) 
                    DASHeldTime = 0.0f;
            }
            else { 
                if (KeysProcessed[Settings.MoveLeft]) 
                {
                    if (Settings.ResetDASOnDirectionChange && PreviousDASDirection == MoveType::DAS_RIGHT) 
                        DASHeldTime = 0.0f;
                    PreviousDASDirection = MoveType::DAS_LEFT;
                    das_direction = MoveType::DAS_LEFT;
                    move_direction = MoveType::MOVE_LEFT;
                }
                else 
                {
                    if (Settings.ResetDASOnDirectionChange && PreviousDASDirection == MoveType::DAS_LEFT) 
                        DASHeldTime = 0.0f;
                    PreviousDASDirection = MoveType::DAS_RIGHT;
                    das_direction = MoveType::DAS_RIGHT;
                    move_direction = MoveType::MOVE_RIGHT;
                }
            }

            if (DASHeldTime >= Settings.DAS) {
                if (Settings.ARR <= 0.0f) 
                {
                    movelist.push_back(das_direction);
                }
                else 
                {
                    if (ARRHeldTime < 0.0f) {
                        // initial move after das activation
                        movelist.push_back(move_direction);
                        ARRHeldTime = 0.0f;
                    }

                    ARRHeldTime += dt;

                    while (ARRHeldTime > Settings.ARR) {
                        movelist.push_back(move_direction);
                        ARRHeldTime -= Settings.ARR;
                    }
                }
            }
        }
        else 
        {
            DASHeldTime = 0.0f;
            ARRHeldTime = -1.0f;
            PreviousDASDirection = MoveType::NO_MOVE;
        }

        if (Keys[Settings.MoveLeft] && !KeysProcessed[Settings.MoveLeft]) 
        {
            movelist.push_back(MoveType::MOVE_LEFT);
            KeysProcessed[Settings.MoveLeft] = true;
        }

        if (Keys[Settings.MoveRight] && !KeysProcessed[Settings.MoveRight]) 
        {
            movelist.push_back(MoveType::MOVE_RIGHT);
            KeysProcessed[Settings.MoveRight] = true;
        }

        if (Keys[Settings.MoveUp] && !KeysProcessed[Settings.MoveUp]) 
        {
            movelist.push_back(MoveType::MOVE_UP);
            KeysProcessed[Settings.MoveUp] = true;
        }

        if (Keys[Settings.MoveDown] && !KeysProcessed[Settings.MoveDown]) 
        {
            movelist.push_back(MoveType::MOVE_DOWN);
            KeysProcessed[Settings.MoveDown] = true;
        }

        if (Keys[Settings.Rotate180] && !KeysProcessed[Settings.Rotate180]) 
        {
            movelist.push_back(MoveType::ROTATE_180);
            KeysProcessed[Settings.Rotate180] = true;
        }

        if (Keys[Settings.RotateAnticlockwise] && !KeysProcessed[Settings.RotateAnticlockwise]) 
        {
            movelist.push_back(MoveType::ROTATE_ANTICLOCKWISE);
            KeysProcessed[Settings.RotateAnticlockwise] = true;
        }

        // vertical movement should have the lowesr precedence to allow 'jumping' over gaps at high gravity / holding down softdrop
        if (Keys[Settings.SoftDrop]) 
        {
            if (KeysProcessed[Settings.SoftDrop]) 
            {
                if (Settings.SDR <= 0.0f) 
                {
                        movelist.push_back(MoveType::SOFTDROP);
                }
                else 
                {
                    SoftDropHeldTime += dt;

                    while (SoftDropHeldTime > 0.0f) {
                        movelist.push_back(MoveType::MOVE_DOWN);
                        SoftDropHeldTime -= Settings.SDR;
                    }
                }
            }
            else 
            {
                SoftDropHeldTime = 0.0f;
                if (Settings.SDR <= 0.0f)
                    movelist.push_back(MoveType::SOFTDROP);
                else
                    movelist.push_back(MoveType::MOVE_DOWN);
                KeysProcessed[Settings.SoftDrop] = true;
            }
        }

        // harddrop and hold should be the last to be processed so that we make sure all buffered moves are executed before spawing the next piece
        if (Keys[Settings.HardDrop] && !KeysProcessed[Settings.HardDrop]) 
        {
            movelist.push_back(MoveType::HARDDROP);
            KeysProcessed[Settings.HardDrop] = true;
        }

        if (Keys[Settings.Hold] && !KeysProcessed[Settings.Hold]) 
        {
            movelist.push_back(MoveType::HOLD);
            KeysProcessed[Settings.Hold] = true;
        }

        if (Keys[Settings.RotateClockwise] && !KeysProcessed[Settings.RotateClockwise]) 
        {
            movelist.push_back(MoveType::ROTATE_CLOCKWISE);
            KeysProcessed[Settings.RotateClockwise] = true;
        }

        if (!movelist.empty()) 
        {
            if (Board->IsPaused) 
                Board->Start();
            Board->ExecuteMoves(movelist);
        }
    }
}

void Game::Render()
{
    if (State == GAME_ACTIVE)
    {
        // draw background
        SpriteRender->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(Width, Height), 0.0f);

        // draw board
        DrawBoard();

        // draw statistics
        DrawStatistics();
    }
}

void Game::DrawBoard()
{
    // draw actual visible board
    SpriteRender->DrawSprite(ResourceManager::GetTexture("back_board"), BOARD_BACK_POS, BOARD_BACK_SIZE);

    // draw minos
    MinoType currentPiece = MinoType::EMPTY;
    for (int i = 0; i < 40; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            currentPiece = Board->Matrix[i][j];
            if (currentPiece != MinoType::EMPTY)
            {
                SpriteRender->DrawSprite(MinoTexture.at(currentPiece), BoardStartPosition + MinoSize * glm::vec2(j, -i), MinoSize, 0.0f, MinoColors.at(currentPiece));
            }
        }
    }

    // draw previews
    int i = 0;
    for (MinoType& type: Board->TetrominoQueue)
    {
        if (i == PREVIEW_NUMBER) break;
        DrawTetrominoPreview(type, i);
        i = i + 1;
    }

    // draw first preview spawn
    DrawTetromino(Board->TetrominoQueue.front(), MinoType::SPAWN_PREVIEW, 0, SpawnPosition);

    // draw current piece
    DrawTetromino(Board->CurrentPiece, Board->CurrentPiece, Board->CurrentRotation, BoardStartPosition + glm::vec2(Board->CurrentPosition.y, -Board->CurrentPosition.x) * MinoSize);

    // draw ghost piece
    // magic number [9] -> shifts the type from the block variant to the ghost variant
    DrawTetromino(Board->CurrentPiece, (MinoType)(Board->CurrentPiece + 9), Board->CurrentRotation, BoardStartPosition + glm::vec2(Board->GhostPosition.y, -Board->GhostPosition.x) * MinoSize);

    // draw held piece
    if (Board->HoldPiece != MinoType::EMPTY)
    {
        if (Board->HoldUsed)
            DrawTetromino(Board->HoldPiece, (MinoType)(Board->HoldPiece + 9), 0, HoldPosition);
        else
            DrawTetromino(Board->HoldPiece, Board->HoldPiece, 0, HoldPosition);
    }
}

void Game::DrawStatistics()
{
    auto elapsedTime = Board->GetElapsedTime();
    unsigned int pieces = Board->PiecesPlaced;
    unsigned int lines = Board->LinesCleared;

    // time:
    std::string milli = std::to_string(elapsedTime.count() - std::floor(elapsedTime.count())).substr(1, 4);
    TextRender->RenderText(std::format("{:%T}", elapsedTime) + milli, StatsStartPosition.x, StatsStartPosition.y, 1.0f);

    // pps:
    if (elapsedTime.count() > 0.0f)
        TextRender->RenderText(std::to_string(pieces / elapsedTime.count()), StatsStartPosition.x, StatsStartPosition.y + StatsSpacing.y * 1.0f, 1.0f);
    else
        TextRender->RenderText(std::to_string(0), StatsStartPosition.x, StatsStartPosition.y + StatsSpacing.y * 1.0f, 1.0f);

    // apm:
    TextRender->RenderText(std::to_string(0), StatsStartPosition.x, StatsStartPosition.y + StatsSpacing.y * 2.0f, 1.0f);

    // lines:
    TextRender->RenderText(std::to_string(lines), StatsStartPosition.x, StatsStartPosition.y + StatsSpacing.y * 3.0f, 1.0f);

    // attack:
    TextRender->RenderText(std::to_string(0), StatsStartPosition.x, StatsStartPosition.y + StatsSpacing.y * 4.0f, 1.0f);

    // app:
    TextRender->RenderText(std::to_string(0), StatsStartPosition.x, StatsStartPosition.y + StatsSpacing.y * 5.0f, 1.0f);

    if (Board->IsOver)
        TextRender->RenderText("Game Over!", Width / 2.0f - 100.0f, Height / 2.0f, 1.0f);

    if (Board->Combo > 1)
        TextRender->RenderText(std::string("Combo x") + std::to_string(Board->Combo), 0, 0, 1.0f);
}

void Game::DrawTetrominoPreview(MinoType type, int previewIndex)
{
    const Tetromino tetromino = Board->RotationOffsets.at(type)[0];
    for (glm::ivec2 offset: tetromino.PieceOffsets)
    {
        SpriteRender->DrawSprite(MinoTexture.at(type), PreviewStartPosition + glm::vec2(0, static_cast<float>(previewIndex * 3)) * MinoSize + MinoSize * glm::vec2(offset.y, -offset.x), MinoSize, 0.0f, MinoColors.at(type));
    }
}

void Game::DrawTetromino(MinoType type, MinoType minoColor, int rotation, glm::vec2 pos)
{
    const Tetromino tetromino = Board->RotationOffsets.at(type)[rotation];
    for (glm::ivec2 offset: tetromino.PieceOffsets)
    {
        SpriteRender->DrawSprite(MinoTexture.at(minoColor), pos + MinoSize * glm::vec2(offset.y, -offset.x), MinoSize, 0.0f, MinoColors.at(minoColor));
    }
}

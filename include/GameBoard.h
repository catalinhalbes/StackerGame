#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include "SpriteRenderer.h"
#include "ResourceManager.h"

#include <glm/glm.hpp>

#include <vector>
#include <array>
#include <list>
#include <string>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <ranges>
#include <chrono>

enum MinoType
{
    EMPTY,
    BLOCK_I,
    BLOCK_J,
    BLOCK_L,
    BLOCK_O,
    BLOCK_S,
    BLOCK_T,
    BLOCK_Z,
    GARBAGE,
    SOLID_GARBAGE,
    GHOST_I,
    GHOST_J,
    GHOST_L,
    GHOST_O,
    GHOST_S,
    GHOST_T,
    GHOST_Z,
    SPAWN_PREVIEW
};

enum MoveType
{
    NO_MOVE,
    MOVE_LEFT,
    MOVE_RIGHT,
    DAS_LEFT,
    DAS_RIGHT,
    ROTATE_CLOCKWISE,
    ROTATE_ANTICLOCKWISE,
    ROTATE_180,
    HARDDROP,
    SOFTDROP,
    HOLD,
    MOVE_UP,
    MOVE_DOWN
};

const int PREVIEW_NUMBER = 6;
const std::array<MinoType, 7> SEVEN_PIECE_BAG = { BLOCK_I, BLOCK_J, BLOCK_L, BLOCK_O, BLOCK_S, BLOCK_T, BLOCK_Z };

struct Tetromino
{
    std::array<glm::ivec2, 4> PieceOffsets;
};

// block rotations
// notation:
// 0: default rotation / spawn state / north orientation
// 1: "R" (right) rotation / east orientation
// 2: 180 rotation / south orientation
// 3: "L" (left) rotation / west orientation
const std::unordered_map<MinoType, std::array<Tetromino, 4>> SRS_TETROMINO_ROTATIONS = {
    { BLOCK_I, {
        Tetromino { { { { 0, -1}, { 0,  0}, { 0,  1}, { 0,  2} } } },
        Tetromino { { { { 1,  1}, { 0,  1}, {-1,  1}, {-2,  1} } } },
        Tetromino { { { {-1, -1}, {-1,  0}, {-1,  1}, {-1,  2} } } },
        Tetromino { { { { 1,  0}, { 0,  0}, {-1,  0}, {-2,  0} } } },
    } },
    { BLOCK_J, {
        Tetromino { { { { 1, -1}, { 0, -1}, { 0,  0}, { 0,  1} } } },
        Tetromino { { { { 1,  0}, { 1,  1}, { 0,  0}, {-1,  0} } } },
        Tetromino { { { { 0, -1}, { 0,  0}, { 0,  1}, {-1,  1} } } },
        Tetromino { { { { 1,  0}, { 0,  0}, {-1, -1}, {-1,  0} } } },
    } },
    { BLOCK_L, {
        Tetromino { { { { 1,  1}, { 0, -1}, { 0,  0}, { 0,  1} } } },
        Tetromino { { { { 1,  0}, { 0,  0}, {-1,  0}, {-1,  1} } } },
        Tetromino { { { { 0, -1}, { 0,  0}, { 0,  1}, {-1, -1} } } },
        Tetromino { { { { 1, -1}, { 1,  0}, { 0,  0}, {-1,  0} } } },
    } },
    { BLOCK_O, {
        Tetromino { { { { 0,  0}, { 0,  1}, { 1,  0}, { 1,  1} } } },
        Tetromino { { { { 0,  0}, { 0,  1}, { 1,  0}, { 1,  1} } } },
        Tetromino { { { { 0,  0}, { 0,  1}, { 1,  0}, { 1,  1} } } },
        Tetromino { { { { 0,  0}, { 0,  1}, { 1,  0}, { 1,  1} } } },
    } },
    { BLOCK_S, {
        Tetromino { { { { 1,  0}, { 1,  1}, { 0, -1}, { 0,  0} } } },
        Tetromino { { { { 1,  0}, { 0,  0}, { 0,  1}, {-1,  1} } } },
        Tetromino { { { { 0,  0}, { 0,  1}, {-1, -1}, {-1,  0} } } },
        Tetromino { { { { 1, -1}, { 0, -1}, { 0,  0}, {-1,  0} } } },
    } },
    { BLOCK_T, {
        Tetromino { { { { 1,  0}, { 0, -1}, { 0,  0}, { 0,  1} } } },
        Tetromino { { { { 1,  0}, { 0,  0}, { 0,  1}, {-1,  0} } } },
        Tetromino { { { { 0, -1}, { 0,  0}, { 0,  1}, {-1,  0} } } },
        Tetromino { { { { 1,  0}, { 0, -1}, { 0,  0}, {-1,  0} } } },
    } },
    { BLOCK_Z, {
        Tetromino { { { { 1, -1}, { 1,  0}, { 0,  0}, { 0,  1} } } },
        Tetromino { { { { 1,  1}, { 0,  0}, { 0,  1}, {-1,  0} } } },
        Tetromino { { { { 0, -1}, { 0,  0}, {-1,  0}, {-1,  1} } } },
        Tetromino { { { { 1,  0}, { 0, -1}, { 0,  0}, {-1, -1} } } },
    } }
};

// kicks
// !BEWARE!
// if tetris wiki uses (x, y) offsets, this code uses (y, x)
const std::vector<std::vector<std::vector<glm::ivec2>>> JLSTZ_SRS_KICK_TABLE {
    {   // 0: spawn / north orientation
        /*0  --->   0*/ { {0, 0} },
        /*0  --->   R*/ { {0, 0}, {0, -1}, {1, -1}, {-2, 0}, {-2, -1} },
        /*0  ---> 180*/ { {0, 0} },
        /*0  --->   L*/ { {0, 0}, {0, 1}, {1, 1}, {-2, 0}, {-2, 1} }
    },
    {   // 1: R / east orientation
        /*R  --->   0*/ { {0, 0}, {0, 1}, {-1, 1}, {2, 0}, {2, 1} },
        /*R  --->   R*/ { {0, 0} },
        /*R  ---> 180*/ { {0, 0}, {0, 1}, {-1, 1}, {2, 0}, {2, 1} },
        /*R  --->   L*/ { {0, 0} }
    },
    {   // 2: 180 / south orientation
        /*180  ->   0*/ { {0, 0} },
        /*180  ->   R*/ { {0, 0}, {0, -1}, {1, -1}, {-2, 0}, {-2, -1} },
        /*180  -> 180*/ { {0, 0} },
        /*180  ->   L*/ { {0, 0}, {0, 1}, {1, 1}, {-2, 0}, {-2, 1} }
    },
    {   // 3: L / west orientation
        /*L  --->   0*/ { {0, 0}, {0, -1}, {-1, -1}, {2, 0}, {2, -1} },
        /*L  --->   R*/ { {0, 0} },
        /*L  ---> 180*/ { {0, 0}, {0, -1}, {-1, -1}, {2, 0}, {2, -1} },
        /*L  --->   L*/ { {0, 0} }
    },
};

const std::vector<std::vector<std::vector<glm::ivec2>>> JLSTZ_SRS_PLUS_KICK_TABLE {
    {   // 0: spawn / north orientation
        /*0  --->   0*/ { {0, 0} },
        /*0  --->   R*/ { {0, 0}, {0, -1}, {1, -1}, {-2, 0}, {-2, -1} },
        /*0  ---> 180*/ { {0, 0}, {1, 0}, {1, 1}, {1, -1}, {0, 1}, {0, -1} },
        /*0  --->   L*/ { {0, 0}, {0, 1}, {1, 1}, {-2, 0}, {-2, 1} }
    },
    {   // 1: R / east orientation
        /*R  --->   0*/ { {0, 0}, {0, 1}, {-1, 1}, {2, 0}, {2, 1} },
        /*R  --->   R*/ { {0, 0} },
        /*R  ---> 180*/ { {0, 0}, {0, 1}, {-1, 1}, {2, 0}, {2, 1} },
        /*R  --->   L*/ { {0, 0}, {0, 1}, {2, 1}, {1, 1}, {2, 0}, {1, 0} }
    },
    {   // 2: 180 / south orientation
        /*180  ->   0*/ { {0, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {0, -1}, {0, 1} },
        /*180  ->   R*/ { {0, 0}, {0, -1}, {1, -1}, {-2, 0}, {-2, -1} },
        /*180  -> 180*/ { {0, 0} },
        /*180  ->   L*/ { {0, 0}, {0, 1}, {1, 1}, {-2, 0}, {-2, 1} }
    },
    {   // 3: L / west orientation
        /*L  --->   0*/ { {0, 0}, {0, -1}, {-1, -1}, {2, 0}, {2, -1} },
        /*L  --->   R*/ { {0, 0}, {0, -1}, {2, -1}, {1, -1}, {2, 0}, {1, 0} },
        /*L  ---> 180*/ { {0, 0}, {0, -1}, {-1, -1}, {2, 0}, {2, -1} },
        /*L  --->   L*/ { {0, 0} }
    },
};

const std::vector<std::vector<std::vector<glm::ivec2>>> O_SRS_KICK_TABLE {
    {   // 0: spawn / north orientation
        /*0  --->   0*/ { {0, 0} },
        /*0  --->   R*/ { {0, 0} },
        /*0  ---> 180*/ { {0, 0} },
        /*0  --->   L*/ { {0, 0} }
    },
    {   // 1: R / east orientation
        /*R  --->   0*/ { {0, 0} },
        /*R  --->   R*/ { {0, 0} },
        /*R  ---> 180*/ { {0, 0} },
        /*R  --->   L*/ { {0, 0} }
    },
    {   // 2: 180 / south orientation
        /*180  ->   0*/ { {0, 0} },
        /*180  ->   R*/ { {0, 0} },
        /*180  -> 180*/ { {0, 0} },
        /*180  ->   L*/ { {0, 0} }
    },
    {   // 3: L / west orientation
        /*L  --->   0*/ { {0, 0} },
        /*L  --->   R*/ { {0, 0} },
        /*L  ---> 180*/ { {0, 0} },
        /*L  --->   L*/ { {0, 0} }
    },
};

const std::vector<std::vector<std::vector<glm::ivec2>>> I_SRS_KICK_TABLE {
    {   // 0: spawn / north orientation
        /*0  --->   0*/ { {0, 0} },
        /*0  --->   R*/ { {0, 0}, {0, -2}, {0, 1}, {-1, -2}, {2, 1} },
        /*0  ---> 180*/ { {0, 0} },
        /*0  --->   L*/ { {0, 0}, {0, -1}, {0, 2}, {2, -1}, {-1, 2} }
    },
    {   // 1: R / east orientation
        /*R  --->   0*/ { {0, 0}, {0, 2}, {0, -1}, {1, 2}, {-2, -1} },
        /*R  --->   R*/ { {0, 0} },
        /*R  ---> 180*/ { {0, 0}, {0, -1}, {0, 2}, {2, -1}, {-1, 2} },
        /*R  --->   L*/ { {0, 0} }
    },
    {   // 2: 180 / south orientation
        /*180  ->   0*/ { {0, 0} },
        /*180  ->   R*/ { {0, 0}, {0, 1}, {0, -2}, {-2, 1}, {1, -2} },
        /*180  -> 180*/ { {0, 0} },
        /*180  ->   L*/ { {0, 0}, {0, 2}, {0, -1}, {1, 2}, {-2, -1} }
    },
    {   // 3: L / west orientation
        /*L  --->   0*/ { {0, 0}, {0, 1}, {0, -2}, {-2, 1}, {1, -2} },
        /*L  --->   R*/ { {0, 0} },
        /*L  ---> 180*/ { {0, 0}, {0, -2}, {0, 1}, {-1, -2}, {2, 1} },
        /*L  --->   L*/ { {0, 0} }
    },
};

const std::vector<std::vector<std::vector<glm::ivec2>>> I_SRS_PLUS_KICK_TABLE {
    {   // 0: spawn / north orientation
        /*0  --->   0*/ { {0, 0} },
        /*0  --->   R*/ { {0, 0}, {0, 1}, {0, -2}, {-1, -2}, {2, 1} },
        /*0  ---> 180*/ { {0, 0}, {1, 0}, {1, 1}, {1, -1}, {0, 1}, {0, -1} },
        /*0  --->   L*/ { {0, 0}, {0, -1}, {0, 2}, {-1, 2}, {2, -1} }
    },
    {   // 1: R / east orientation
        /*R  --->   0*/ { {0, 0}, {0, -1}, {0, 2}, {-2, -1}, {1, 2} },
        /*R  --->   R*/ { {0, 0} },
        /*R  ---> 180*/ { {0, 0}, {0, -1}, {0, 2}, {2, -1}, {-1, 2} },
        /*R  --->   L*/ { {0, 0}, {0, 1}, {2, 1}, {1, 1}, {2, 0}, {1, 0} }
    },
    {   // 2: 180 / south orientation
        /*180  ->   0*/ { {0, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {0, -1}, {0, 1} },
        /*180  ->   R*/ { {0, 0}, {0, -2}, {0, -1}, {1, -2}, {-2, 1} },
        /*180  -> 180*/ { {0, 0} },
        /*180  ->   L*/ { {0, 0}, {0, 2}, {0, -1}, {1, 2}, {-2, -1} }
    },
    {   // 3: L / west orientation
        /*L  --->   0*/ { {0, 0}, {0, 1}, {0, -2}, {-2, 1}, {1, -2} },
        /*L  --->   R*/ { {0, 0}, {0, -1}, {2, -1}, {1, -1}, {2, 0}, {1, 0} },
        /*L  ---> 180*/ { {0, 0}, {0, 1}, {0, -2}, {2, 1}, {-1, -2} },
        /*L  --->   L*/ { {0, 0} }
    },
};

const std::unordered_map<MinoType, std::vector<std::vector<std::vector<glm::ivec2>>>> SRS_KICK_TABLE = {
    { BLOCK_I, I_SRS_KICK_TABLE},
    { BLOCK_J, JLSTZ_SRS_KICK_TABLE},
    { BLOCK_L, JLSTZ_SRS_KICK_TABLE},
    { BLOCK_O, O_SRS_KICK_TABLE},
    { BLOCK_S, JLSTZ_SRS_KICK_TABLE},
    { BLOCK_T, JLSTZ_SRS_KICK_TABLE},
    { BLOCK_Z, JLSTZ_SRS_KICK_TABLE}
};

const std::unordered_map<MinoType, std::vector<std::vector<std::vector<glm::ivec2>>>> SRS_PLUS_KICK_TABLE = {
    { BLOCK_I, I_SRS_PLUS_KICK_TABLE},
    { BLOCK_J, JLSTZ_SRS_PLUS_KICK_TABLE},
    { BLOCK_L, JLSTZ_SRS_PLUS_KICK_TABLE},
    { BLOCK_O, O_SRS_KICK_TABLE},
    { BLOCK_S, JLSTZ_SRS_PLUS_KICK_TABLE},
    { BLOCK_T, JLSTZ_SRS_PLUS_KICK_TABLE},
    { BLOCK_Z, JLSTZ_SRS_PLUS_KICK_TABLE}
};

class GameBoard
{
    public:
        std::unordered_map<MinoType, std::array<Tetromino, 4>> RotationOffsets;
        std::unordered_map<MinoType, std::vector<std::vector<std::vector<glm::ivec2>>>> KickTable;

        // game  state
        MinoType Matrix[40][10];        // internal state of the board
        std::list<MinoType> TetrominoQueue;

        MinoType CurrentPiece, HoldPiece;
        glm::ivec2 CurrentPosition, GhostPosition;
        int CurrentRotation;
        bool HoldUsed;

        unsigned int LinesCleared, PiecesPlaced, Combo;
        std::chrono::time_point<std::chrono::high_resolution_clock> StartTime, StopTime;

        unsigned int BagSeed;
        std::mt19937 BagRNG;

        bool IsOver, IsPaused;

        // constructor
        GameBoard(const std::unordered_map<MinoType, std::array<Tetromino, 4>>& rotationOffsets, const std::unordered_map<MinoType, std::vector<std::vector<std::vector<glm::ivec2>>>>& kickTable);

        // prepares board
        void Load();

        // set matrix configuration
        // works with all sizes equal or smaller than the matrix used
        void SetMatrix(const std::vector<std::vector<MinoType>>& matrix);

        void ExecuteMoves(const std::list<MoveType>& moves);

        std::chrono::duration<double> GetElapsedTime();

        void Start();
        void Stop();

    private:
        void ClearBoard();
        std::array<MinoType, 7> GetNextBag();
        void PopulateQueue();
        glm::ivec2 SoftDropPosition();
        bool CurrentPieceCanMoveAt(glm::ivec2 position, int rotation);
        void NextPiece();
        unsigned int ClearLines();
        bool RotateWithKick(MoveType rot);
};

#endif // GAMEBOARD_H

#ifndef GAME_H
#define GAME_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "ResourceManager.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "GameBoard.h"
#include "GameSettings.h"

#include <unordered_map>
#include <string>
#include <chrono>
#include <format>
#include <cmath>

enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

class Game
{
    public:
        static const glm::vec2 BOARD_SIZE;
        static const glm::vec2 BOARD_POS;
        static const glm::vec2 BOARD_BACK_POS;
        static const glm::vec2 BOARD_BACK_SIZE;
        static const float GHOST_OPACITY;

        const GameSettings&     Settings;
        GameState               State;
        bool                    Keys[1024];
        bool                    KeysProcessed[1024];
        float                   DASHeldTime;
        float                   ARRHeldTime;
        float                   SoftDropHeldTime;
        MoveType                PreviousDASDirection;
        unsigned int            Width, Height;

        SpriteRenderer          *SpriteRender;
        TextRenderer            *TextRender;
        GameBoard               *Board;

        glm::vec2               BoardSize;
        glm::vec2               BoardPosition;
        glm::vec2               BoardStartPosition;
        glm::vec2               PreviewStartPosition;
        glm::vec2               HoldPosition;
        glm::vec2               SpawnPosition;
        glm::vec2               MinoSize;

        glm::vec2               StatsStartPosition;
        glm::vec2               StatsSpacing;

        std::unordered_map<MinoType, glm::vec4> MinoColors;
        std::unordered_map<MinoType, Texture2D> MinoTexture;

        Game(unsigned int width, unsigned int height, const GameSettings& settings);
        ~Game();

        void Init();

        // game loop
        void ProcessInput(float dt);
        void Update(float dt);
        void Render();

    private:
        void DrawBoard();
        void DrawStatistics();
        void DrawTetrominoPreview(MinoType type, int previewIndex);
        void DrawTetromino(MinoType type, MinoType minoColor, int rotation, glm::vec2 pos);
};

#endif // GAME_H

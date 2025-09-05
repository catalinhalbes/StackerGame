#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include <toml++/toml.h>
#include <string>
#include <GLFW/glfw3.h>

class GameSettings {
public:
    int MoveLeft, MoveRight, MoveUp, MoveDown;
    int SoftDrop, HardDrop;
    int Hold;
    int RotateClockwise, RotateAnticlockwise, Rotate180;
    int Restart, Quit;
    double DAS, ARR, SDR;
    bool ResetDASOnDirectionChange;

    GameSettings(const std::string& filename);

private:
    static int ConvertToGlfwScancode(const std::string& key);
};

#endif // GAME_SETTINGS_H
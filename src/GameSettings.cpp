#include "GameSettings.h"
#include <iostream>
#include <unordered_map>
#include <exception>

GameSettings::GameSettings(const std::string& filename) {
    toml::table settings;

    settings = toml::parse_file(filename);

    std::cout << settings << std::endl;

    MoveLeft                    = ConvertToGlfwScancode(settings["Keybinds"]["move_left"].value_or<std::string>(""));
    MoveRight                   = ConvertToGlfwScancode(settings["Keybinds"]["move_right"].value_or<std::string>(""));
    MoveUp                      = ConvertToGlfwScancode(settings["Keybinds"]["move_up"].value_or<std::string>(""));
    MoveDown                    = ConvertToGlfwScancode(settings["Keybinds"]["move_down"].value_or<std::string>(""));
    SoftDrop                    = ConvertToGlfwScancode(settings["Keybinds"]["soft_drop"].value_or<std::string>(""));
    HardDrop                    = ConvertToGlfwScancode(settings["Keybinds"]["hard_drop"].value_or<std::string>(""));
    Hold                        = ConvertToGlfwScancode(settings["Keybinds"]["hold"].value_or<std::string>(""));
    RotateClockwise             = ConvertToGlfwScancode(settings["Keybinds"]["rotate_clockwise"].value_or<std::string>(""));
    RotateAnticlockwise         = ConvertToGlfwScancode(settings["Keybinds"]["rotate_anticlockwise"].value_or<std::string>(""));
    Rotate180                   = ConvertToGlfwScancode(settings["Keybinds"]["rotate_180"].value_or<std::string>(""));
    Restart                     = ConvertToGlfwScancode(settings["Keybinds"]["restart"].value_or<std::string>(""));

    DAS                         = settings["Movement"]["DAS"].value_or<float>(0.0);
    ARR                         = settings["Movement"]["ARR"].value_or<float>(0.0);
    SDR                         = settings["Movement"]["SDR"].value_or<float>(0.0);
    ResetDASOnDirectionChange   = settings["Movement"]["DAS_cancel"].value_or<bool>(true);
}

int GameSettings::ConvertToGlfwScancode(const std::string& key) {
    static std::map<std::string, int> keyMap = {
        {"a", GLFW_KEY_A}, {"b", GLFW_KEY_B}, {"c", GLFW_KEY_C},
        {"d", GLFW_KEY_D}, {"e", GLFW_KEY_E}, {"f", GLFW_KEY_F},
        {"g", GLFW_KEY_G}, {"h", GLFW_KEY_H}, {"i", GLFW_KEY_I},
        {"j", GLFW_KEY_J}, {"k", GLFW_KEY_K}, {"l", GLFW_KEY_L},
        {"m", GLFW_KEY_M}, {"n", GLFW_KEY_N}, {"o", GLFW_KEY_O},
        {"p", GLFW_KEY_P}, {"q", GLFW_KEY_Q}, {"r", GLFW_KEY_R},
        {"s", GLFW_KEY_S}, {"t", GLFW_KEY_T}, {"u", GLFW_KEY_U},
        {"v", GLFW_KEY_V}, {"w", GLFW_KEY_W}, {"x", GLFW_KEY_X},
        {"y", GLFW_KEY_Y}, {"z", GLFW_KEY_Z},

        {"0", GLFW_KEY_0}, {"1", GLFW_KEY_1}, {"2", GLFW_KEY_2},
        {"3", GLFW_KEY_3}, {"4", GLFW_KEY_4}, {"5", GLFW_KEY_5},
        {"6", GLFW_KEY_6}, {"7", GLFW_KEY_7}, {"8", GLFW_KEY_8},
        {"9", GLFW_KEY_9},

        {"arrow_left", GLFW_KEY_LEFT}, {"arrow_right", GLFW_KEY_RIGHT},
        {"arrow_up", GLFW_KEY_UP}, {"arrow_down", GLFW_KEY_DOWN},

        {"space", GLFW_KEY_SPACE}, 
        {"enter", GLFW_KEY_ENTER},
        {"left_shift", GLFW_KEY_LEFT_SHIFT}, 
        {"left_ctrl", GLFW_KEY_LEFT_CONTROL},
        {"right_shift", GLFW_KEY_RIGHT_SHIFT}, 
        {"right_ctrl", GLFW_KEY_RIGHT_CONTROL},
        {"left_alt", GLFW_KEY_LEFT_ALT}, 
        {"right_alt", GLFW_KEY_RIGHT_ALT}, 
        {"tab", GLFW_KEY_TAB},
        {"caps", GLFW_KEY_CAPS_LOCK}
    };

    auto it = keyMap.find(key);

    if (it == keyMap.end()) {
        throw std::runtime_error((std::string("Key not found: ") + key).c_str());
    }

    return it->second;
}
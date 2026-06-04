#include "Input.h"
#include <string>
#include <unordered_map>
#pragma once

namespace InputConverter {
    const BYTE ConvertKeyToDIK(const std::string& keyName);
    const std::string ConvertDIKToKey(const BYTE dik);

    const Controller ConvertKeyToController(const std::string& keyName);
    const std::string ConvertControllerToKey(const Controller controller);

    const DPad ConvertKeyToDPad(const std::string& keyName);
    const std::string ConvertDPadToKey(const DPad dPad);

    inline const std::unordered_map<std::string, BYTE> m_dikTable = {
        {"A", DIK_A},
        {"B", DIK_B},
        {"C", DIK_C},
        {"D", DIK_D},
        {"E", DIK_E},
        {"F", DIK_F},
        {"G", DIK_G},
        {"H", DIK_H},
        {"I", DIK_I},
        {"J", DIK_J},
        {"K", DIK_K},
        {"L", DIK_L},
        {"M", DIK_M},
        {"N", DIK_N},
        {"O", DIK_O},
        {"P", DIK_P},
        {"Q", DIK_Q},
        {"R", DIK_R},
        {"S", DIK_S},
        {"T", DIK_T},
        {"U", DIK_U},
        {"V", DIK_V},
        {"W", DIK_W},
        {"X", DIK_X},
        {"Y", DIK_Y},
        {"Z", DIK_Z},
        {"0", DIK_0},
        {"1", DIK_1},
        {"2", DIK_2},
        {"3", DIK_3},
        {"4", DIK_4},
        {"5", DIK_5},
        {"6", DIK_6},
        {"7", DIK_7},
        {"8", DIK_8},
        {"9", DIK_9},
        {"SPACE", DIK_SPACE },
        {"ENTER", DIK_RETURN },
        {"LSHIFT", DIK_LSHIFT },
        {"RSHIFT", DIK_RSHIFT },
        {"LCONTROL", DIK_LCONTROL },
        {"RCONTROL", DIK_RCONTROL },
        {"ESCAPE", DIK_ESCAPE },
        {"LEFT", DIK_LEFT },
        {"RIGHT", DIK_RIGHT },
        {"UP", DIK_UP },
        {"DOWN", DIK_DOWN },
        {"TAB", DIK_TAB },
        {"BACK", DIK_BACK },
        {"CAPSLOCK", DIK_CAPITAL },
        {"F1", DIK_F1 },
        { "F2", DIK_F2 },
        { "F3", DIK_F3 },
        { "F4", DIK_F4 },
        { "F5", DIK_F5 },
        { "F6", DIK_F6 },
        { "F7", DIK_F7 },
        { "F8", DIK_F8 },
        { "F9", DIK_F9 },
        { "F10", DIK_F10 },
        { "F11", DIK_F11 },
        { "F12", DIK_F12 },
        {"Comma", DIK_COMMA },
        {"Period", DIK_PERIOD },
        {"Slash", DIK_SLASH },
        {"Apostrophe", DIK_APOSTROPHE },
        {"LeftBracket", DIK_LBRACKET },
        {"RightBracket", DIK_RBRACKET },
    };
    inline const std::unordered_map<std::string, Controller> m_controllerTable = {
        {"None", Controller::None},
        {"A", Controller::A},
        {"B", Controller::B},
        {"X", Controller::X},
        {"Y", Controller::Y},
        {"LB", Controller::LB},
        {"RB", Controller::RB},
        {"LT", Controller::LT},
        {"RT", Controller::RT},
        {"View", Controller::View},
        {"Menu", Controller::Menu},
        {"LeftStick", Controller::LeftStick},
        {"RightStick", Controller::RightStick},
    };
    inline const std::unordered_map<std::string, DPad> m_DPadTable = {
        {"None", DPad::None},
        {"Up", DPad::Up},
        {"UpRight", DPad::UpRight},
        {"UpLeft", DPad::UpLeft},
        {"Down", DPad::Down},
        {"DownRight", DPad::DownRight},
        {"DownLeft", DPad::DownLeft},
        {"Left", DPad::Left},
        {"Right", DPad::Right},
    };
}

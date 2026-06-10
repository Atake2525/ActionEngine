#include "Input.h"
#include <unordered_map>
#include <array>
#include <string>
#include "SettingUtility.h"
#pragma once

namespace Setting {

    class KeyboardConfig
    {
    public:
        KeyboardConfig() = default;
        ~KeyboardConfig() = default;

        void SetAction(Action action, BYTE key) { m_keyboard[action] = key; }
        const int GetAction(Action action) const;
        const int GetDefaultAction(Action action) const { return m_defaultKeyboard.at(action); }
    private:
        std::unordered_map<Action, int> m_keyboard;
        std::unordered_map<Action, int> m_defaultKeyboard = {
            {Action::MoveForward, DIK_W },
            { Action::MoveBack, DIK_S },
            { Action::MoveLeft, DIK_A },
            { Action::MoveRight, DIK_D },
            { Action::Jump, DIK_SPACE },
            { Action::Crouch, DIK_LCONTROL },
            { Action::Run, DIK_LSHIFT },
            { Action::Interact, DIK_F },
        };
    };
}

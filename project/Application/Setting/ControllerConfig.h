#include "SettingUtility.h"
#include "Input.h"
#include <array>
#include <unordered_map>
#pragma once

namespace Setting {

    class ControllerConfig {
    public:
        void SetAction(Action action, Controller controller);
        void SetAction(Action action, DPad dpad);
        void SetAction(Action action, StickDirection stickDirection);
        const Controller GetControllerAction(Action action) const;
        const DPad GetDPadAction(Action action) const;
        const StickDirection GetStickAction(Action action) const;
    private:
        std::unordered_map<Action, Controller> m_controller;
        std::unordered_map<Action, DPad> m_dpad;
        std::unordered_map<Action, StickDirection> m_stickDirection;
    };
}

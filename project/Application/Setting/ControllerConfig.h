#include "SettingUtility.h"
#include "Input.h"
#include <array>
#include <unordered_map>
#pragma once

namespace Setting {

    class ControllerConfig {
    public:
        void SetInput(Input* input) { m_pInput = input; }
        void SetAction(Action action, Controller controller);
        void SetAction(Action action, DPad dpad);
        const Controller GetControllerAction(Action action) const;
        const DPad GetDPadAction(Action action) const;
    private:
        Input* m_pInput = nullptr;
        std::unordered_map<Action, Controller> m_controller;
        std::unordered_map<Action, DPad> m_dpad;
    };
}
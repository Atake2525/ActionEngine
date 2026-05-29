#include "SettingUtility.h"
#include "Input.h"
#include <array>
#include <unordered_map>
#pragma once

namespace Setting {

    class ControllerConfig {
    public:
        bool Push(Action action);
        bool Trigger(Action action);
        bool Return(Action action);

        void SetInput(Input* input) { m_pInput = input; }
        void SetAction(Action action, Controller controller);
    private:
        Input* m_pInput = nullptr;
        std::unordered_map<Action, Controller> m_controller;
    };
}
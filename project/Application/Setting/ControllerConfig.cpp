#include "ControllerConfig.h"

void Setting::ControllerConfig::SetAction(Action action, Controller controller) {
    m_controller[action] = controller;
}

void Setting::ControllerConfig::SetAction(Action action, DPad dpad) {
    m_dpad[action] = dpad;
}

const Controller Setting::ControllerConfig::GetControllerAction(Action action) const
{
    auto it = m_controller.find(action);
    if (it == m_controller.end()) // 未設定の場合は0を返す
    {
        return Controller::None;
    }
    return it->second;
}

const DPad Setting::ControllerConfig::GetDPadAction(Action action) const
{
    auto it = m_dpad.find(action);
    if (it == m_dpad.end()) // 未設定の場合は0を返す
    {
        return DPad::None;
    }
    return it->second;
}
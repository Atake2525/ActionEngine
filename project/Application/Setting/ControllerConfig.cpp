#include "ControllerConfig.h"

void Setting::ControllerConfig::SetAction(Action action, Controller controller) {
    m_controller[action] = controller;
}

void Setting::ControllerConfig::SetAction(Action action, DPad dpad) {
    m_dpad[action] = dpad;
}

void Setting::ControllerConfig::SetAction(Action action, StickDirection stickDirection) {
    m_stickDirection[action] = stickDirection;
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

const Setting::StickDirection Setting::ControllerConfig::GetStickAction(Action action) const
{
    auto it = m_stickDirection.find(action);
    if (it == m_stickDirection.end())
    {
        return StickDirection::None;
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

const Controller Setting::ControllerConfig::GetDefaultControllerAction(Action action) const
{
    auto it = m_defaultController.find(action);
    if (it == m_defaultController.end()) // 未設定の場合は0を返す
    {
        return Controller::None;
    }
    return it->second;
}

const DPad Setting::ControllerConfig::GetDefaultDPadAction(Action action)
{
    auto it = m_defaultDPad.find(action);
    if (it == m_defaultDPad.end()) // 未設定の場合は0を返す
    {
        return DPad::None;
    }
    return it->second;
}

const Setting::StickDirection Setting::ControllerConfig::GetDefaultStickDirectionAction(Action action)
{
    auto it = m_defaultStickDirection.find(action);
    if (it == m_defaultStickDirection.end()) // 未設定の場合は0を返す
    {
        return StickDirection::None;
    }
    return it->second;
}
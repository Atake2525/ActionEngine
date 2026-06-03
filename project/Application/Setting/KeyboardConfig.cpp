#include "KeyboardConfig.h"

using namespace Setting;

bool Setting::KeyboardConfig::Push(Action action) const
{
    for (int i = 0; i < 2; i++)
    {
        auto it = m_keyboard[i].find(action);
        if (it == m_keyboard[i].end()) // 未設定の場合はfalseを返す
        {
            return false;
        }
        int key = it->second;
        if (m_pInput->PushKeyInt(key) && (key > 0 || key <= 256))
        {
            return true;
        }
    }
    return false;
}

bool Setting::KeyboardConfig::Trigger(Action action) const
{
    for (int i = 0; i < 2; i++)
    {
        auto it = m_keyboard[i].find(action);
        if (it == m_keyboard[i].end()) // 未設定の場合はfalseを返す
        {
            return false;
        }
        int key = it->second;
        if (m_pInput->TriggerKeyInt(key) && (key > 0 || key <= 256))
        {
            return true;
        }
    }
    return false;
}

bool Setting::KeyboardConfig::Return(Action action) const
{
    for (int i = 0; i < 2; i++)
    {
        auto it = m_keyboard[i].find(action);
        if (it == m_keyboard[i].end()) // 未設定の場合はfalseを返す
        {
            return false;
        }
        int key = it->second;
        if (m_pInput->ReturnKeyInt(key) && (key > 0 || key <= 256))
        {
            return true;
        }
    }
    return false;
}
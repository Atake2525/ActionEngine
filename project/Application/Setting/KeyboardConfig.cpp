#include "KeyboardConfig.h"

using namespace Setting;

const int Setting::KeyboardConfig::GetAction(Action action) const
{
    auto it = m_keyboard.find(action);
    if (it == m_keyboard.end()) // 未設定の場合は-1を返す
    {
        return -1;
    }
    return it->second;
}
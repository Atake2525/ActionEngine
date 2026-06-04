#include "KeyboardConfig.h"

using namespace Setting;

const int Setting::KeyboardConfig::GetMainAction(Action action) const
{
    auto it = m_keyboard[0].find(action);
    if (it == m_keyboard[0].end()) // 未設定の場合は-1を返す
    {
        return -1;
    }
    return it->second;
}
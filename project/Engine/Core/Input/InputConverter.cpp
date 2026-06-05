#include "InputConverter.h"


namespace InputConverter {
    const BYTE ConvertKeyToDIK(const std::string& keyName) {
        if (m_dikTable.contains(keyName))
        {
            return m_dikTable.at(keyName);
        }
        return -1;
    }
    const std::string ConvertDIKToKey(const BYTE dik) {
        for (const auto& pair : m_dikTable)
        {
            if (pair.second == dik)
            {
                return pair.first;
            }
        }
        return "null";
    }
    const Controller ConvertKeyToController(const std::string& keyName) {
        if (m_controllerTable.contains(keyName))
        {
            return m_controllerTable.at(keyName);
        }
        return Controller::None;
    }
    const std::string ConvertControllerToKey(const Controller controller) {
        for (const auto& pair : m_controllerTable)
        {
            if (pair.second == controller)
            {
                return pair.first;
            }
        }
        return "null";
    }
    const DPad ConvertKeyToDPad(const std::string& keyName) {
        if (m_DPadTable.contains(keyName))
        {
            return m_DPadTable.at(keyName);
        }
        return DPad::None;
    }
    const std::string ConvertDPadToKey(const DPad dPad) {
        for (const auto& pair : m_DPadTable)
        {
            if (pair.second == dPad)
            {
                return pair.first;
            }
        }
        return "null";
    }

    const Setting::StickDirection ConvertKeyToStickDirection(const std::string& keyName) {
        if (m_stickDirectionTable.contains(keyName))
        {
            return m_stickDirectionTable.at(keyName);
        }
        return Setting::StickDirection::None;
    }

    const std::string ConvertStickDirectionToKey(const Setting::StickDirection stickDirection) {
        for (const auto& pair : m_stickDirectionTable)
        {
            if (pair.second == stickDirection)
            {
                return pair.first;
            }
        }
        return "null";
    }
} // namespace InputConverter

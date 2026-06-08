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

		void SetMainAction(Action action, BYTE key) { m_keyboard[0][action] = key; }
		const int GetMainAction(Action action) const;
		void SetSubAction(Action action, BYTE key) { m_keyboard[1][action] = key; }
	private:
		Input* m_pInput = nullptr;
		std::array<std::unordered_map<Action, int>, 2> m_keyboard;
		std::unordered_map<std::string, int> KeyNameToDik;
	};
}

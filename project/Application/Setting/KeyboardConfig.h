#include "Input.h"
#include <unordered_map>
#include <array>
#include <string>
#include "SettingUtility.h"
#pragma once

namespace Setting {

	class KeyConfig
	{
	public:
		KeyConfig();
		~KeyConfig();

		bool Push(Action action) const;
		bool Trigger(Action action) const;
		bool Return(Action action) const;

		void SetInput(Input* input) { m_pInput = input; }
		void SetMainAction(Action action, BYTE key) { m_keyboard[0][action] = key; }
		void SetSubAction(Action action, BYTE key) { m_keyboard[1][action] = key; }
	private:
		Input* m_pInput = nullptr;
		std::array<std::unordered_map<Action, int>, 2> m_keyboard;
		std::unordered_map<std::string, int> KeyNameToDik;
	};
}
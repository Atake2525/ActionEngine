#include <optional>
#include <string>
#include <unordered_map>
#include <array>
#pragma once

namespace Setting {

	struct Sensitivity
	{
		float mouse = 1.0f;
        float controller = 1.0f;
        bool invertX = false;
        bool invertY = false;
	};

	enum class Action
	{
		MoveForward,
		MoveBack,
		MoveLeft,
		MoveRight,
		Jump,
		Crouch,
		Run,
		Interact,
	};

	enum class StickDirection
	{
		None,
		LeftStickUp,
		LeftStickDown,
		LeftStickLeft,
		LeftStickRight,
		RightStickUp,
		RightStickDown,
		RightStickLeft,
		RightStickRight,
	};
	inline const std::array<std::pair<std::string, Action>, 8> ActionNameToEnum = { {
				{ "MoveForward", Action::MoveForward },
				{ "MoveBack",    Action::MoveBack },
				{ "MoveLeft",    Action::MoveLeft },
				{ "MoveRight",   Action::MoveRight },
				{ "Jump",        Action::Jump },
				{ "Crouch",      Action::Crouch },
				{ "Run",         Action::Run },
				{ "Interact",    Action::Interact },
	} };

	inline std::optional<Action> ToAction(const std::string name) {
		const auto it = std::find_if(ActionNameToEnum.begin(), ActionNameToEnum.end(), [&name](const auto& pair) {
			return pair.first == name;
		});
		if (it == ActionNameToEnum.end())
		{
			return std::nullopt;
		}

		return it->second;
	}

    inline const std::string ToString(const Action action) {
        for (const auto& pair : ActionNameToEnum)
        {
            if (pair.second == action)
            {
                return pair.first;
            }
        }
        return "null";
    }
}

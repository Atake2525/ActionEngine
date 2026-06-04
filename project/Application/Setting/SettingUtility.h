#include <optional>
#include <string>
#include <unordered_map>
#pragma once

namespace Setting {
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
	const std::unordered_map<std::string, Action> ActionNameToEnum = {
				{ "MoveForward", Action::MoveForward },
				{ "MoveBack",    Action::MoveBack },
				{ "MoveLeft",    Action::MoveLeft },
				{ "MoveRight",   Action::MoveRight },
				{ "Jump",        Action::Jump },
				{ "Crouch",      Action::Crouch },
				{ "Run",         Action::Run },
				{ "Interact",    Action::Interact },
	};

	std::optional<Action> ToAction(const std::string name) {
		const auto it = ActionNameToEnum.find(name);
		if (it == ActionNameToEnum.end())
		{
			return std::nullopt;
		}

		return it->second;
	}
}
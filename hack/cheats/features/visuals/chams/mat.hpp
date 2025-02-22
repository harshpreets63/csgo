#pragma once

#include <string>

class IMaterial;

struct Mat_t
{
	struct Data
	{
		std::string name{};
		std::string key{};
		std::string buf{};
	};

	enum class ExtraType
	{
		NONE,
		GLOW = 1
	};

	enum class StrategyType
	{
		BUFFER,
		FROM_STRING
	};

	bool isFromEditor{ false }; // editor
	ExtraType type{ ExtraType::NONE };
	StrategyType strategy{ StrategyType::BUFFER }; // editor
	Data data{};
	IMaterial* mat{};

	constexpr IMaterial* operator->() const { return mat; }
	constexpr bool operator!() const { return mat == nullptr; }
};
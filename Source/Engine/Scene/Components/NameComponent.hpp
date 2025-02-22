#pragma once

#include "Core/String.hpp"

namespace lde
{
	struct NameComponent
	{
		NameComponent() = default;
		NameComponent(std::string_view Name)
			: Name(Name)
		{ }

		std::string Name;

	};
}
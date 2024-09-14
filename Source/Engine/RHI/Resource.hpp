#pragma once

// TODO:

namespace lde
{
	class Device;

	class Resource
	{
	public:

		virtual void* GetHandle() = 0;

	protected:
		Device* m_Device = nullptr;

	};
} // namespace lde

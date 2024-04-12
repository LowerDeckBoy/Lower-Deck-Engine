#pragma once

// TODO:

namespace lde::RHI
{
	class Device;

	class Resource
	{
	public:

		virtual void* GetHandle() = 0;

	protected:
		Device* m_Device = nullptr;

	};
} // namespace lde::RHI

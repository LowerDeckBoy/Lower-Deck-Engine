#pragma once

/*
	Core/Singleton.hpp
	Singleton wrapper for manager-like classes.
*/

namespace mf
{
	template<typename T>
	class Singleton
	{
	protected:
		Singleton() = default;
		Singleton(const Singleton&) = delete;
		Singleton(const Singleton&&) = delete;
		Singleton operator=(const Singleton&) = delete;
		Singleton operator=(const Singleton&&) = delete;

	public:
		static T& GetInstance()
		{
			static T* instance = nullptr;
			if (!instance)
			{
				instance = new T();
			}

			return *instance;
		}

	};
} // namespace mf

#pragma once

/*
	Core/Singleton.hpp
	Singleton wrapper for manager-like classes.
*/

namespace lde
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

		static Singleton* Instance;
	public:
		static T& GetInstance()
		{
			static T* instance; // = nullptr
			if (!instance)
			{
				instance = new T();
			}

			return *instance;
		}

	};
} // namespace lde

#pragma once

/*
	Core/RefPtr.hpp
	Replacement for Microsoft::WRL::ComPtr.
*/

#include <concepts>
#include <memory>

namespace mf
{
	template<typename T>
	concept TInterface = requires(T * Ptr)
	{
		Ptr->AddRef();
		Ptr->Release();
	};

	template<TInterface T>
	class Ref
	{
	public:
		Ref() : m_Ptr(nullptr) {}
		Ref(T* Interface) : m_Ptr(new T)
		{
			AddRef();
		}

		Ref(const Ref& Other) : m_Ptr(Other.m_Ptr)
		{
			AddRef();
		}

		Ref(Ref&& Other) noexcept
			: m_Ptr(std::exchange(Other.m_Ptr, nullptr))
		{
		}

		~Ref()
		{
			InternalRelease();
		}

		T* Get() const
		{
			return m_Ptr;
		}

		Ref& operator=(T* Ptr) noexcept
		{
			if (this->m_Ptr != Ptr)
			{
				Ref(m_Ptr).Swap(*this);
			}
			return *this;
		}

		Ref& operator=(const Ref& Other) noexcept
		{
			if (this != &Other)
			{
				if (this->m_Ptr != Other.m_Ptr)
				{
					Ref(Other).Swap(*this);
				}
			}
			return *this;
		}

		Ref& operator=(Ref&& Other) noexcept
		{
			Ref(std::forward<Ref>(Other)).Swap(*this);
			return *this;
		}

		//T* const* GetAddressOf()
		T** GetAddressOf()
		{
			return &m_Ptr;
		}

		T** ReleaseAndGetAddressOf()
		{
			InternalRelease();
			return &m_Ptr;
		}

		T& operator=(std::nullptr_t) noexcept
		{
			InternalRelease();
			m_Ptr = nullptr;
			return *m_Ptr;
		}

		T* operator->() const
		{
			return m_Ptr;
		}

		T** operator&()
		{
			return &m_Ptr;
		}

		void Swap(Ref& Other)
		{
			std::swap(m_Ptr, Other.m_Ptr);
		}

		void Swap(Ref&& Other)
		{
			std::swap(m_Ptr, Other.m_Ptr);
		}

		unsigned long Reset()
		{
			return InternalRelease();
		}

		/// @brief Manual release.
		void Release()
		{
			if (m_Ptr) delete m_Ptr;
		}
	private:
		T* m_Ptr{ nullptr };

		void AddRef()
		{
			if (m_Ptr)
				m_Ptr->AddRef();
		}

		unsigned long InternalRelease()
		{
			unsigned long RefCount{ 0 };
			if (T* Temp = m_Ptr; Temp)
			{
				m_Ptr = nullptr;
				RefCount = Temp->Release();
			}
			return RefCount;
		}
	};
}

#pragma once

//#include <EnTT/entt.hpp>
#include "World.hpp"

namespace lde
{
//class World;

	class Entity
	{
		friend class World;
	public:
		Entity() = default;
		Entity(World* pWorld) : m_World(pWorld)
		{
			m_ID = pWorld->CreateEntity();
		}
		Entity(World* pWorld, entt::entity ID)
			: m_World(pWorld), m_ID(ID)
		{ }
		~Entity()
		{
			//if (m_World) m_World->DestroyEntity(Handle);
			//m_World = nullptr;
		}
	
		void Create(World* pWorld)
		{
			if (IsAlive())
				return;
	
			m_ID = pWorld->CreateEntity();
			m_World = pWorld;
		}
	
		inline entt::entity ID() const
		{
			return m_ID;
		}
	
		bool IsAlive()
		{
			return m_World != nullptr;
		}
	
		template<typename T>
		T& GetComponent()
		{
			return m_World->Registry()->get<T>(m_ID);
		}
	
		template<typename T, typename... Args>
		void AddComponent(Args&&... InArgs)
		{
			m_World->Registry()->emplace<T>(this->m_ID, std::forward<Args>(InArgs)...);
		}
	
		template<typename T>
		void RemoveComponent()
		{
			m_World->Registry()->remove<T>(m_ID);
		}
	
		template<typename T>
		bool HasComponent()
		{
			return m_World->Registry()->any_of<T>(m_ID);
		}
	
		bool IsValid()
		{
			return m_World->Registry()->valid(m_ID);
		}
	
		bool operator==(const Entity& Other) const
		{
			return (m_ID == Other.m_ID) && (m_World == Other.m_World);
		}
	
		bool operator!=(const Entity& Other)
		{
			return !(*this == Other);
		}
	
	private:
		/// @brief Reference to World object that THIS entity belongs to.
		World* m_World = nullptr;
	
		/// @brief Handle of Entity id
		entt::entity m_ID{};
	
	};

} // namespace lde

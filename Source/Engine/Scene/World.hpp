#pragma once

/*
	
*/

#include "EnTT/entt.hpp"

namespace lde
{
	class World
	{
		friend class Entity;
	public:
		/// @brief Initializes Registry.
		World()
		{
			m_Registry = new entt::registry();
		}
		World(const World&) = delete;
		World(const World&&) = delete;
		/// @brief Releases Registry.
		~World()
		{
			m_Registry->clear();
			delete m_Registry;
		}
	
		/// @brief 
		/// @return Pointer to underlying Registry. 
		entt::registry* Registry()
		{
			return m_Registry;
		}
	
		/// @brief 
		/// @return ID of newly created Entity. 
		entt::entity CreateEntity()
		{
			return m_Registry->create();
		}
	
		/// @brief Removes entity from Registry.
		/// @param Entity Entity to remove.
		void DestroyEntity(entt::entity Entity)
		{
			m_Registry->destroy(Entity);
		}
	
	private:
		entt::registry* m_Registry = nullptr;
	
	};

} // namespace lde

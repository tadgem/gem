#pragma once
#include "json.hpp"
#include "hash_string.h"
#include <vector>
#include <unordered_map>
#include <type_traits>
class scene;

class ecs_system
{
public:

	virtual void			init() = 0;
	virtual void			update(scene& current_scene) = 0;
	virtual void			cleanup() = 0;

	virtual nlohmann::json	serialize(scene& current_scene) = 0;
	virtual void			deserialize(scene& current_scene, nlohmann::json& sys_json) = 0;

    virtual ~ecs_system() {}
};

class system_manager
{
public:
	std::vector<std::unique_ptr<ecs_system>>		m_systems;
	std::unordered_map<hash_string_ge, ecs_system*>	m_system_type_aliases;

	template<typename _SysType, typename... Args>
	ecs_system* add_system(Args&&... args)
	{
		static_assert(std::is_base_of<ecs_system, _SysType>());
		hash_string type_str_hash = hash_string{ hash_utils::get_type_hash<_SysType>() };
		m_systems.push_back(std::make_unique<_SysType>());
		m_system_type_aliases.emplace(type_str_hash, m_systems.back().get());
		return m_system_type_aliases[type_str_hash];
	}

};
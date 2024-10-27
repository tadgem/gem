#pragma once
#include "json.hpp"

class scene;

class ecs_system
{
public:

	virtual void			init() = 0;
	virtual void			update(scene& current_scene) = 0;
	virtual void			cleanup() = 0;

	virtual nlohmann::json	        serialize(scene& current_scene) = 0;
	virtual void			deserialize(scene& current_scene) = 0;

        virtual ~ecs_system() {}
};
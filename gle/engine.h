#pragma once
#include "backend.h"
#include "asset_manager.h";
#include "scene.h"
#include "renderer.h"
#include "events.h"
#include "ecs_system.h"

class engine
{
public:
	inline static asset_manager								assets;
	inline static event_handler								events;
	inline static gl_renderer								renderer;
	inline static scene_manager								scenes;
	inline static std::vector<std::unique_ptr<ecs_system>>	                                systems;
};
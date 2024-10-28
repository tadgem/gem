#pragma once
#include "backend.h"
#include "asset_manager.h";
#include "scene.h"
#include "renderer.h"
#include "events.h"
#include "ecs_system.h"
#include "project.h"


class engine
{
public:
	inline static asset_manager									assets;
	inline static event_handler									events;
	inline static gl_renderer									renderer;
	inline static scene_manager									scenes;
	inline static std::vector<std::unique_ptr<ecs_system>>	    systems;
	inline static project										active_project;

	static void   save_project_to_disk(const std::string& filename, const std::string& directory);
	static void   load_project_from_disk(const std::string& filepath);
};
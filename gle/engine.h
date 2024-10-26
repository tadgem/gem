#pragma once
#include "backend.h"
#include "asset_manager.h";
#include "scene.h"
#include "renderer.h"
#include "events.h"

class engine
{
public:
	inline static asset_manager			assets;
	inline static event_handler			events;
	inline static gl_renderer			renderer;
	inline static scene_manager			scenes;
};
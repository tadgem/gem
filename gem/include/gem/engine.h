#pragma once
#include "asset_manager.h";
#include "backend.h"
#include "ecs_system.h"
#include "events.h"
#include "project.h"
#include "renderer.h"
#include "scene.h"

struct debug_callbacks_collection
{
  std::vector<std::function<void()>> m_callbacks;

  void add(std::function<void()> debug_callback);
};

namespace gem {

class engine {
public:
  inline static asset_manager assets;
  inline static event_handler events;
  inline static gl_renderer renderer;
  inline static scene_manager scenes;
  inline static system_manager systems;
  inline static project active_project;
  inline static debug_callbacks_collection debug_callbacks;

  static void init();
  static void update();
  static void save_project_to_disk(const std::string &filename,
                                   const std::string &directory);
  static void load_project_from_disk(const std::string &filepath);
  static void shutdown();
};

} // namespace gem
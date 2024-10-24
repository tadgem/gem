#pragma once
#include "gle.h"
#include "editor_definitions.h"
#include "editor_app.h" 
#include <sstream>
#include <iostream>

class editor_application
{
public:
	editor_application();

	asset_manager				m_asset_manager;
	// todo: store a const reference to a base renderer class to allow for vk backend
	gl_renderer_builtin			m_renderer;

	fsm							m_editor_fsm;

	debug_camera_controller		m_editor_camera_controller;
	camera						m_editor_camera;

	void run();

protected:
	void main_menu_bar();
};
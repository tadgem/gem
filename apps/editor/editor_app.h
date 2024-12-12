#pragma once
#include "gem/gem.h"
#include "gem/project.h"
#include <sstream>
#include <iostream>
#include "editor_definitions.h"

class editor_application
{
public:
	editor_application();

	gem::fsm							m_editor_fsm;
	gem::DebugCameraController m_editor_camera_controller;
	gem::Camera m_editor_camera;

	void run();

	void on_open_project();
	void on_edit();
	void on_play();
	void main_menu_bar();
	
	gem::Project create_project(const std::string& name, const std::string& path);
};
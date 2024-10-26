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

	fsm							m_editor_fsm;

	debug_camera_controller		m_editor_camera_controller;
	camera						m_editor_camera;

	void run();

	void on_open_project();
	void on_edit();
	void on_play();
	void main_menu_bar();
};
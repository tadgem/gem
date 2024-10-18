#include "renderer.h"

void renderer::init()
{
	for (int i = 0; i < m_render_passes.size(); i++)
	{
		m_render_passes[i]->init();
	}
}

void renderer::pre_update(scene& current_scene)
{
	for (int i = 0; i < m_render_passes.size(); i++)
	{
		m_render_passes[i]->pre_update(current_scene);
	}
}

void renderer::frame(scene& current_scene)
{
	for (int i = 0; i < m_render_passes.size(); i++)
	{
		m_render_passes[i]->dispatch(current_scene);
	}
}

void renderer::cleanup()
{
	for (int i = 0; i < m_render_passes.size(); i++)
	{
		m_render_passes[i]->cleanup();
	}
}

void built_in_renderer_pass::init()
{
}

void built_in_renderer_pass::pre_update(scene& current_scene)
{
}

void built_in_renderer_pass::dispatch(scene& current_scene)
{
}

void built_in_renderer_pass::cleanup()
{
}

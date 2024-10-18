#pragma once
#include <memory>
#include "framebuffer.h"
#include "shader.h"
#include "texture.h"

class scene;

class render_pass
{
public:
	virtual void init() = 0;
	virtual void pre_update(scene& current_scene) = 0;
	virtual void dispatch(scene& current_scene) = 0;
	virtual void cleanup() = 0;
};

class renderer
{
public:
	std::vector<std::unique_ptr<render_pass>> m_render_passes;

	void init();
	void pre_update(scene& current_scene);
	void frame(scene& current_scene); // aka post update
	void cleanup();

};

class built_in_renderer_pass : public render_pass
{
public:
	void init() override;
	void pre_update(scene& current_scene) override;
	void dispatch(scene& current_scene) override;
	void cleanup() override;
};
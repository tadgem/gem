#pragma once

namespace agpu
{

	class shader
	{

	};

	// instance of interface to gpu driver (vulkan or gl)
	class adapter
	{
	public:
		
		virtual void	init() = 0;
		virtual void	cleanup() = 0;
		
		// accepts the shader source as arguments
		virtual shader	create_shader(const std::string& vertex, const std::string& fragment) = 0;
		virtual shader	create_compute_shader(const std::string& compute) = 0;

	};

}
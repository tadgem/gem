#pragma once
#include <string>
#include "GL/glew.h"

class GL_DEBUG
{
public:
	GL_DEBUG(const std::string& name)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name.c_str());
	}

	~GL_DEBUG()
	{
		glPopDebugGroup();
	}
};

#define GPU_MARKER(X) GL_DEBUG __marker__(X)
#pragma once
#define ENABLE_PROFILING
#ifdef ENABLE_PROFILING
#else
#undef TRACY_ENABLE
#endif
#include "GL/glew.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"
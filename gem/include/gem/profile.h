#pragma once
// TODO(LiamD) : This leaks memory when enabled and no profiler is connected.
// #define ENABLE_PROFILING
#ifdef ENABLE_PROFILING && !TRACY_ENABLE
#define TRACY_ENABLE
#else
#undef TRACY_ENABLE
#endif
#include "GL/glew.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"
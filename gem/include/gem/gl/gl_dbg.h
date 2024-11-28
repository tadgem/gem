#pragma once
#include "GL/glew.h"
#include <string>

namespace gem {

class GL_DEBUG {
public:
  GL_DEBUG(const std::string &name) {
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name.c_str());
  }

  ~GL_DEBUG() { glPopDebugGroup(); }
};

#define GPU_MARKER(X) GL_DEBUG __marker__(X)
} // namespace gem


GLenum glCheckError_(const char *file, int line);
#define glAssert(X)                                                            \
  X;                                                                           \
  glCheckError_(__FILE__, __LINE__)
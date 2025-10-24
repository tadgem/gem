#pragma once
#include "gem/mesh.h"
namespace gem {
class GLMesh : public AMesh {
  VAO m_vao;

  void Draw() { m_vao.draw(); }
};
}
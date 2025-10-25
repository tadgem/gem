#pragma once
#include "gem/mesh.h"
namespace gem {
class GLMesh : public AMesh {
  VAO vao;

  void Draw() { vao.Draw(); }
};
}
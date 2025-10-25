#pragma once
#include "asset.h"
#include "gem/gl/gl_shader.h"
#include "model.h"
#include "texture.h"

namespace gem {
using GLShaderAsset = TAsset<GLShader, AssetType::kShader>;
using TextureAsset = TAsset<Texture, AssetType::kTexture>;
using ModelAsset = TAsset<Model, AssetType::kModel>;
} // namespace gem
#pragma once
#include "asset.h"
#include "gem/gl/gl_shader.h"
#include "model.h"
#include "texture.h"

namespace gem {
using GLShaderAsset = TAsset<GLShader, AssetType::shader>;
using TextureAsset = TAsset<Texture, AssetType::texture>;
using ModelAsset = TAsset<Model, AssetType::model>;
} // namespace gem
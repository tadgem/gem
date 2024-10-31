#pragma once
#include "asset.h"
#include "model.h"
#include "shader.h"
#include "texture.h"

namespace gem {
using shader_asset = asset_t<shader, asset_type::shader>;
using texture_asset = asset_t<texture, asset_type::texture>;
using model_asset = asset_t<model, asset_type::model>;
} // namespace gem
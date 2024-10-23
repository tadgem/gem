#pragma once
#include "asset.h"
#include "shader.h"
#include "model.h"
#include "texture.h"

using shader_asset = asset_t<shader, asset_type::shader>;
using texture_asset = asset_t<texture, asset_type::texture>;
using model_asset = asset_t<model, asset_type::model>;

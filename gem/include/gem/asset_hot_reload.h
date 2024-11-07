//
// Created by liam_ on 11/7/2024.
//

#pragma once
#include "asset.h"

class shader;

namespace gem
{
  void reload_shader( asset_t<shader, asset_type::shader>* shader_asset,
                      std::string shader_source);

}
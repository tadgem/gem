#include "gem/shader.h"
#include "gem/profile.h"=
#include <iostream>
#include <sstream>

std::unordered_map<gem::Shader::stage, std::string>
gem::Shader::split_composite_shader(const std::string &input) {
  ZoneScoped;
  static std::unordered_map<std::string, Shader::stage> s_known_stages = {
      {"#frag", Shader::stage::fragment},
      {"#vert", Shader::stage::vertex},
      {"#geom", Shader::stage::geometry},
      {"#compute", Shader::stage::compute}};

  auto stages = std::unordered_map<Shader::stage, std::string>();
  std::string version = "";

  std::stringstream input_stream(input);
  std::stringstream stage_stream{};
  std::string line = "";
  std::string stage = "";

  while (std::getline(input_stream, line)) {
    if (line.find("#version") != std::string::npos) {
      version = line;
      stage_stream << version << "\n";
      continue;
    }

    for (auto &[known, stage_enum] : s_known_stages) {
      if (line.find(known) != std::string::npos) {
        if (!stage.empty()) {
          stages.emplace(Shader::stage(s_known_stages[stage]),
                         std::string(stage_stream.str()));
          stage_stream.str(std::string());
          stage_stream.clear();
          stage_stream << version << "\n";
        }

        stage = known;
        break;
      }
    }

    if (line == stage) {
      continue;
    }

    stage_stream << line << "\n";
  }

  std::string last_stream = stage_stream.str();
  if (!stage.empty() && !last_stream.empty()) {
    stages.emplace(s_known_stages[stage], last_stream);
  }

  return stages;
}

#include "gem/shader.h"
#include "gem/profile.h"=
#include <iostream>
#include <sstream>

std::unordered_map<gem::Shader::Stage, std::string>
gem::Shader::SplitCompositeShader(const std::string &input) {
  ZoneScoped;
  static std::unordered_map<std::string, Shader::Stage> s_known_stages = {
      {"#frag", Shader::Stage::kFragment},
      {"#vert", Shader::Stage::kVertex},
      {"#geom", Shader::Stage::kGeometry},
      {"#compute", Shader::Stage::kCompute}};

  auto stages = std::unordered_map<Shader::Stage, std::string>();
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
          stages.emplace(Shader::Stage(s_known_stages[stage]),
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

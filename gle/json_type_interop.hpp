#pragma once

#include "json.hpp"
#include "glm.hpp"
#include "entt.hpp"

namespace glm
{
    void to_json(nlohmann::json& j, const glm::vec2& v2) {
        j = nlohmann::json{ {"x", v2.x}, {"y", v2.y}};
    }

    void to_json(nlohmann::json& j, const glm::vec3& v3) {
        j = nlohmann::json{ {"x", v3.x}, {"y", v3.y}, {"z", v3.z}};
    }

    void to_json(nlohmann::json& j, const glm::vec4& v4) {
        j = nlohmann::json{ {"x", v4.x}, {"y", v4.y}, {"z", v4.z} , {"w", v4.w}};
    }

    void from_json(const nlohmann::json& j, glm::vec2& v2) {
        j.at("x").get_to(v2.x);
        j.at("y").get_to(v2.y);
    }

    void from_json(const nlohmann::json& j, glm::vec3& v3) {
        j.at("x").get_to(v3.x);
        j.at("y").get_to(v3.y);
        j.at("z").get_to(v3.z);
    }

    void from_json(const nlohmann::json& j, glm::vec4& v4) {
        j.at("x").get_to(v4.x);
        j.at("y").get_to(v4.y);
        j.at("z").get_to(v4.z);
        j.at("w").get_to(v4.w);
    }
}

namespace entt
{
    void to_json(nlohmann::json& j, const entity& e) {
        j = nlohmann::json{ {"entity", static_cast<u32>(e)}};
    }

    void from_json(const nlohmann::json& j, entity& e) {
        u32 index = 0;
        j.at("entity").get_to(index);
        e = static_cast<entt::entity>(index);
    }

}
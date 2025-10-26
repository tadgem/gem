#include "gem/gem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "gem/gl/gl_backend.h"
#include "gem/material.h"
#include "gem/mesh.h"
#include "gem/profile.h"
#include "gem/transform.h"
#include "gem/utils.h"

namespace gem {

    // define global impls in engine tu
    AssetManager Assets = AssetManager();
    EventHandler Events = EventHandler();
    GLRenderer Renderer = GLRenderer();
    SceneCollection Scenes = SceneCollection();
    SystemCollection Systems = SystemCollection();
    Project ActiveProject = Project();
    DebugCallbackCollection DebugCallbacks = DebugCallbackCollection();

    void Init(const glm::ivec2& resolution) {
        ZoneScoped;
        BackendInit init_props = { resolution, true };
        GPUBackend::InitBackend<GLBackend>(init_props);

        Systems.AddSystem<TransformSystem>();
        Systems.AddSystem<MeshSystem>();
        Systems.AddSystem<MaterialSystem>();

        Renderer.Init(Assets, resolution);
    }

    void SaveProjectToDisk(const std::string& filename,
        const std::string& directory) {
        ZoneScoped;
        std::string final_path = directory + "/" + filename;
        Utils::SaveStringToPath(final_path,
            ActiveProject.Serialize(Assets).dump());
    }

    void LoadProjectFromDisk(const std::string& filepath) {
        ZoneScoped;
        nlohmann::json proj_json =
            nlohmann::json::parse(Utils::LoadStringFromPath(filepath));
        Project new_proj{};
        new_proj.Deserialize(Assets, proj_json);
        ActiveProject = new_proj;
    }

    void Update() {
        Assets.Update();

        for (auto& cb : DebugCallbacks.callbacks) {
            cb();
        }
        DebugCallbacks.callbacks.clear();
    }
} // namespace gem

void DebugCallbackCollection::Add(std::function<void()> debug_callback) {
    callbacks.emplace_back(debug_callback);
}

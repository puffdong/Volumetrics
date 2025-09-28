#include "ResourceManager.hpp"

ResourceManager::ResourceManager(const std::string& assets_root_path, const std::string& assets_handle)
: root_path(assets_root_path), asset_handle(assets_handle)
{

}

std::string ResourceManager::get_full_path(const std::string& asset_path) {
    return std::string(root_path) + std::string(asset_path.substr(asset_handle.size(), asset_path.size()));
}

uint64_t ResourceManager::generate_new_id() {
    const uint64_t id = UUID{}.value(); // what if collision? oh well, problem for another day
    return id;
}

Resource ResourceManager::load_shader(const std::string& shader_asset_path) {
    uint64_t resource_id = generate_new_id();
    shader_map[resource_id] = new Shader(get_full_path(shader_asset_path));

    Resource s_res = { resource_id, shader_asset_path, SHADER }; // res_id, opengl_id, type
    resource_map[resource_id] = std::move(s_res); // keep resource thing in here
    return s_res;
}

std::optional<Shader*> ResourceManager::get_shader(uint64_t resource_id) {
    if (resource_map.find(resource_id) != resource_map.end()) {
        Resource res = resource_map[resource_id];
        if (res.type == SHADER) { 
            return shader_map[res.id]; 
        } else {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }
}
#include "ResourceManager.hpp"
#include <iostream>

ResourceManager::ResourceManager(const std::string& assets_root_path, const std::string& assets_handle)
: root_path(assets_root_path), asset_handle(assets_handle)
{

}

std::string ResourceManager::get_full_path(const std::string& asset_path) {
    return std::string(root_path) + std::string(asset_path.substr(asset_handle.size(), asset_path.size()));
}

ResourceID ResourceManager::generate_new_id() {
    return UUID<Resource>{};
}

Resource ResourceManager::load_shader(const std::string& shader_asset_path) {
    ResourceID resource_id = generate_new_id();
    shader_map[resource_id] = std::make_unique<Shader>(get_full_path(shader_asset_path));

    Resource s_res = { resource_id, shader_asset_path, ResourceType::Shader };
    resource_map[resource_id] = std::move(s_res); // keep resource thing in here

    return s_res;
}

Resource ResourceManager::load_shader(const std::string& vertex_asset_path, const std::string& fragment_asset_path) {
    ResourceID resource_id = generate_new_id();
    shader_map[resource_id] = std::make_unique<Shader>(get_full_path(vertex_asset_path), get_full_path(fragment_asset_path)); 

    // didn't account for the possibility of having multiple paths for what is semantically one resource, eeehhhh....
    Resource s_res = { resource_id, vertex_asset_path, ResourceType::Shader }; // let's just save the vertex_path for now xd
    resource_map[resource_id] = std::move(s_res);

    return s_res;
}

std::optional<Shader*> ResourceManager::get_shader(ResourceID resource_id) {
    if (resource_map.find(resource_id) != resource_map.end()) {
        Resource res = resource_map[resource_id];
        if (res.type == ResourceType::Shader) { 
            return shader_map[res.id].get(); 
        } else {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }
}
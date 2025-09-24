#include "ResourceManager.hpp"

ResourceManager::ResourceManager(const std::string& assets_root_path, const std::string& assets_handle)
: root_path(assets_root_path), asset_handle(assets_handle)
{

}

std::string ResourceManager::get_full_path(const std::string& asset_path) {
    return std::string(root_path) + std::string(asset_path.substr(asset_handle.size(), asset_path.size()));
}

int ResourceManager::generate_new_id() { // its crude, but hey, it gets the work done for now
    number_of_resources += 1;
    return number_of_resources; // gon be a pain to get this working with multithreading down the line
    // fuck, what if we remove something? 
    // eh, u know what, there is a reason we abstract this in the first place,
    // currently nothing actually gets deleted during run-time so we won't have any issue as of yet
}

Resource ResourceManager::load_shader(const std::string& shader_asset_path) {
    int resource_id = generate_new_id();
    shader_map[resource_id] = new Shader(get_full_path(shader_asset_path));

    Resource s_res = { resource_id, shader_asset_path, SHADER }; // res_id, opengl_id, type
    resoruce_map[resource_id] = std::move(s_res); // keep resource thing in here
    return s_res;
}

std::optional<Shader*> ResourceManager::get_shader(int resource_id) {
    if (resoruce_map.find(resource_id) != resoruce_map.end()) {
        Resource res = resoruce_map[resource_id];
        if (res.type == SHADER) { 
            return shader_map[res.id]; 
        } else {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }
}
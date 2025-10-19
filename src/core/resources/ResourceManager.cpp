#include "ResourceManager.hpp"
#include <optional>
#include <string>
#include <memory>
#include <iostream>

#include "core/rendering/Shader.hpp"
#include "core/rendering/Model.hpp"
#include "core/rendering/Texture.hpp"

#include "core/resources/adapters/ShaderAdapter.hpp"

// internal resource representations
// struct TextureResource {
//     ResourceID id;
//     unsigned int gl_id;
//     int width, height, depth;
// };

// struct ModelResource {
//     ResourceID id;
//     unsigned int gl_id;
// };

// shader resource specification!

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

std::optional<Res::Shader> ResourceManager::new_load_shader(const std::string& vertex_asset_path, const std::string& fragment_asset_path) {
    auto optional_shader = ShaderAdapter::load_shader(get_full_path(vertex_asset_path), get_full_path(fragment_asset_path));
    if (optional_shader) {
        ShaderResource shader = *optional_shader;
        
        Res::Shader r_shader;
        r_shader.id = shader.id;
        r_shader.name = shader.name;
        r_shader.vertex_asset_path = vertex_asset_path;
        r_shader.fragment_asset_path = fragment_asset_path;
        
        
        std::cout << "Shader Load name: " << shader.name << std::endl;
        std::cout << "Shader Load ID: " << shader.id << std::endl;
        std::cout << "Shader Load gl_id: " << shader.rendering_id << std::endl;
        std::cout << "Shader Load v_path: " << shader.vertex_file_path << std::endl;
        std::cout << "Shader Load f_path: " << shader.fragment_file_path << std::endl;
        new_shader_map[shader.id] = std::move(shader);
        
        return r_shader;
    }
    
    return std::nullopt;
}
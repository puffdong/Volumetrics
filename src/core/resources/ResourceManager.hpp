#pragma once
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <unordered_set>
#include "core/UUID.hpp"

// - - - Todo - - - //
// enable shader unloading 
// setting uniforms (template version would be cool) 
// re-implement uniform cache support 
// hot-reloading 
// couple the ResourceManager and Renderer a bit more tho (friend?!)

// Outward Facing
namespace Res {
    struct Shader {
        ShaderID id;
        std::string name;
        std::string vertex_asset_path;
        std::string fragment_asset_path;
    };

    struct Texture {
        TextureID id;
    };

    struct Model {
        ModelID id;
    };
}

// internal resource representations
struct ShaderResource {
    ShaderID id;
    unsigned int rendering_id;
    std::unordered_map<std::string, int> uniform_location_cache;
	std::unordered_map<std::string, int> uniform_block_index_cache;
   
    std::string name;
    std::string vertex_file_path;
    std::string fragment_file_path;
    std::filesystem::file_time_type shader_last_changed;
};

using ResourceID = UUID<Resource>;

enum class ResourceType { Shader, Texture, Model, None };

struct Resource {
    ResourceID id;
    std::string asset_path = "";
    ResourceType type = ResourceType::None;
};

class ResourceManager {    
private:
    // internal resource representations
    // struct TextureResource; 
    // struct ModelResource;

    std::string root_path;
    std::string asset_handle;

    // std::unordered_set<ResourceID> active_resource_ids;
    std::unordered_map<ResourceID, Resource, uuid_hash<Resource>> resource_map;
    std::unordered_map<ResourceID, std::unique_ptr<Shader>, uuid_hash<Resource>> shader_map;
    std::unordered_set<int> resource_id_set;
    int number_of_resources = 0;

    // revamping it
    std::unordered_map<ShaderID, ShaderResource, uuid_hash<Res::Shader>> new_shader_map;


public:
    ResourceManager(const std::string& assets_root_path, const std::string& assets_handle = "res://");
    std::string get_full_path(const std::string& asset_path);
    
    Resource load_shader(const std::string& shader_asset_path);
    Resource load_shader(const std::string& vertex_asset_path, const std::string& fragment_asset_path);
    
    // I am going to abstract this thing below later but need it for now heh
    // The plan is to not even have "shaders" be a thing and any uniform and such is going to just be 
    // abstracted away blah blah, we working on it yuh yuh
    std::optional<Shader*> get_shader(UUID<Resource> resource_id);
    
    // new loaders
    std::optional<Res::Shader> new_load_shader(const std::string& vertex_asset_path, const std::string& fragment_asset_path);
    
private:
    ResourceID generate_new_id();



};
#pragma once
// OpenGL stuff
#include <GL/glew.h>

// std libs
#include <optional>
#include <filesystem>  
#include <vector>
#include <string>
#include <unordered_map>

// Core
#include "core/rendering/Shader.hpp"
#include "core/rendering/Model.hpp"
#include "core/rendering/Texture.hpp"

class ResourceManager {    
private:
    std::string root_path;
    std::string asset_handle;

    std::unordered_map<int, GLuint> shader_map;

public:
    ResourceManager(const std::string& assets_root_path, const std::string& assets_handle = "res://");
    std::string get_full_path(const std::string& asset_path);
    int load_shader(const std::string& filepath);
    
private:



};
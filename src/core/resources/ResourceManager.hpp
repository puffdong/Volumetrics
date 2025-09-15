#pragma once
// OpenGL stuff
#include <GL/glew.h>

// std libs
#include <vector>
#include <string>
#include <filesystem>  
#include <unordered_map>
// #include <map> // hmm gotta get wifi cuz i don't remember off the top of my head

// Core
#include "core/rendering/Shader.hpp"
#include "core/rendering/Model.hpp"
#include "core/rendering/Texture.hpp"

class ResourceManager {    
public: 
    ResourceManager();
    // GLuint load_shader(const std::string& filepath);
    // GLuint load_texture(const std::string& filepath);
    // GLuint load_model(const std::string& filepath);
    
private: 
    std::vector<GLuint> shaders;
    std::vector<GLuint> textures;
    std::vector<GLuint> models;

};
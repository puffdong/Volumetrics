#include "Object.hpp"

Object::Object(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, Base* parent,
           const std::string& shader_path, 
           const std::string& model_path, 
           const std::string& texture_path) 
           : Base(pos, rot, scale, parent) 
           {
            r_shader.asset_path = shader_path;
            r_model.asset_path = model_path;
            r_texture.asset_path = texture_path;
           }

void Object::init(ResourceManager& resources, uint64_t id) {

}

void Object::tick(float delta, ButtonMap bm) {

}

void Object::enqueue(Renderer& renderer, ResourceManager& resources) {
    
}
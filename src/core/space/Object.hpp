#pragma once
#include "glm/glm.hpp"
#include "core/utils/ButtonMap.hpp"
#include "core/rendering/Renderer.hpp"
#include "core/resources/ResourceManager.hpp"
#include "core/UUID.hpp"
#include "core/rendering/Shader.hpp"

// Material properties for rendering
// Matches the layout in the shader's b_material_block
struct Material {
    glm::vec4 diffuse_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // rgb + diffuse strength
    glm::vec4 specular_color = glm::vec4(1.0f, 1.0f, 1.0f, 0.6f); // rgb + specular strength
    glm::vec4 params = glm::vec4(64.0f, 0.0f, 0.0f, 0.0f);        // shininess, metallic, roughness, padding
};

class Object {
private:
    UUID<Object> _id;
    std::string _name = "";
    
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    
    bool _visible = true;
    bool _cast_shadows = true;
    bool _active = true;
    bool _selected = false;
    
    Resource r_shader;
    Shader* shader = nullptr;

    Res::Model r_model;
    Resource r_texture;
    
    Material material; // Material properties for rendering

public:
    Object(glm::vec3 pos = glm::vec3(0.f),
           glm::vec3 rot = glm::vec3(0.f),
           glm::vec3 scale = glm::vec3(1.f), 
           const std::string& shader_path = "",
           const std::string& model_path = "",
           const std::string& texture_path = "");
    
    virtual ~Object();
    
    void init(ResourceManager& resources, const std::string& name = "");
    void tick(float delta);
    void enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos, glm::vec3 sun_dir, glm::vec4 sun_color);

    // getters n' setters
    UUID<Object> get_id() const { return _id; };
    std::string get_name() const { return _name; };
    glm::mat4 get_model_matrix() const;
    glm::vec3 get_position() const { return position; };
    glm::vec3 get_rotation() const { return rotation; };
    glm::vec3 get_scale() const { return scale; };
    bool is_visible() const { return _visible; };
    bool is_active() const { return _active; };
    bool is_selected() const { return _selected; };
    bool casts_shadows() const { return _cast_shadows; };
    void set_cast_shadows(const bool v) { _cast_shadows = v; };
    void set_position(const glm::vec3& p) { position = p; };
    void set_rotation(const glm::vec3& r) { rotation = r; };
    void set_scale(const glm::vec3& s) { scale = s; };
    void set_visibility(const bool v) { _visible = v; };
    void set_active(const bool v) { _active = v; };
    void set_selected(const bool v) { _selected = v; };
    void set_name(const std::string& name) { _name = name; };

    void set_model(Res::Model res);
    
    // Material access
    Material& get_material() { return material; }
    const Material& get_material() const { return material; }
    void set_material(const Material& mat) { material = mat; }
};
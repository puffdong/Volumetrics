#include "Object.hpp"
#include <iostream>

Object::Object(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale,
           const std::string& shader_path, 
           const std::string& model_path, 
           const std::string& texture_path) 
           : position(pos), rotation(rot), scale(scale), 
           _shader_path(shader_path), _model_path(model_path), _texture_path(texture_path) {
            _id = UUID<Object>{};
        }

Object::~Object() {}

void Object::init(ResourceManager& resources, const std::string& name) {
    _name = name.empty() ? "O_" + std::to_string(_id.value()) : name;
    
    if (_model_path.empty()) {
        r_model = resources.load_model("res://models/cube.obj"); // default to cube, source engine "error" inspired :3
    } else {
        r_model = resources.load_model(_model_path);
    }
    
    shader = new Shader(resources.get_full_path(_shader_path), resources.get_full_path(_shader_path.substr(0, _shader_path.find_last_of('.'))+".fs"));
}

void Object::tick(float delta) {

}

glm::mat4 Object::get_model_matrix() const {
    glm::mat4 m(1.0f);
    m = glm::translate(m, position);
    m = glm::rotate(m, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // roll
    m = glm::rotate(m, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // yaw
    m = glm::rotate(m, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // pitch
    m = glm::scale(m, scale);
    return m;
}

void Object::enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos, glm::vec3 sun_dir, glm::vec4 sun_color) {
    glm::mat4 proj = renderer.get_proj();
    glm::mat4 view = renderer.get_view();
    glm::mat4 model = get_model_matrix();
    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model)));
    
    shader->hot_reload_if_changed();
    shader->bind();
    
    // Set material uniforms
    shader->set_uniform_vec4("u_diffuse_color", material.diffuse_color);
    shader->set_uniform_vec4("u_specular_color", material.specular_color);
    shader->set_uniform_vec4("u_material_params", material.params);
    shader->set_uniform_int("u_is_selected", _selected ? 1 : 0);
    shader->set_uniform_vec3("u_sun_dir", sun_dir);
    shader->set_uniform_vec4("u_sun_color", sun_color);
    shader->set_uniform_mat4("u_mvp", proj * view * model);
    shader->set_uniform_mat4("u_proj", renderer.get_proj());
    shader->set_uniform_mat4("u_view", view);
    shader->set_uniform_mat3("u_normal_matrix", normal_matrix);
    shader->set_uniform_mat4("u_light_space_matrix", renderer.get_light_space_matrix());

    TextureBinding shadow_bind{ renderer.get_shadow_map_texture_id(), GL_TEXTURE_2D, 5, "u_shadow_map" };

    auto gpu_model = resources.get_model_gpu_data(r_model);
    
    RenderCommand cmd{};
    cmd.vao = gpu_model.vao;
    cmd.draw_type = DrawType::Elements;
    cmd.count = gpu_model.index_count;
    cmd.shader = shader;
    cmd.textures.push_back(shadow_bind);
    cmd.model_matrix = model;
    cmd.attach_lights = true;

    renderer.submit(RenderPass::Forward, cmd);

    if (_cast_shadows) {
        RenderCommand shadow_cmd{};
        shadow_cmd.vao = gpu_model.vao;
        shadow_cmd.draw_type = DrawType::Elements;
        shadow_cmd.count = gpu_model.index_count;
        shadow_cmd.shader = renderer.get_shadow_shader(); // todo: streamline this, why append the shader in which the renderer already owns?
        shadow_cmd.attach_lights = false;
        shadow_cmd.model_matrix = model;
        renderer.submit(RenderPass::Shadow, shadow_cmd);
    }
}

void Object::set_model(Res::Model res) {
    r_model = res;
}

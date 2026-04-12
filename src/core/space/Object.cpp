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

void Object::enqueue(Renderer& renderer, ResourceManager& resources) {
    if (!_visible) return;

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
    shader->set_uniform_mat4("u_proj", renderer.get_proj());
    shader->set_uniform_mat4("u_view", view);
    shader->set_uniform_mat3("u_normal_matrix", normal_matrix);
    shader->set_uniform_mat4("u_light_space_matrix", renderer.get_light_space_matrix());

    TextureBinding shadow_bind{ renderer.get_shadow_map_texture_id(), GL_TEXTURE_2D, 5, "u_shadow_map" };

    const auto& gpu_model = resources.get_model_gpu_data(r_model);

    for (const auto& instance : gpu_model.instances) {
        const Mesh& mesh = gpu_model.meshes[instance.mesh_index];
        const glm::mat4 world_model = model * instance.transform;
        const glm::mat3 world_normal_matrix = glm::transpose(glm::inverse(glm::mat3(world_model)));
        shader->set_uniform_mat4("u_mvp", proj * view * world_model);
        shader->set_uniform_mat3("u_normal_matrix", world_normal_matrix);

        for (const auto& primitive : mesh.primitives) {
            RenderCommand cmd{};
            cmd.vao = primitive.vao;
            cmd.draw_type = primitive.index_count > 0 ? DrawType::Elements : DrawType::Arrays;
            cmd.count = primitive.index_count > 0 ? primitive.index_count : primitive.vertex_count;
            cmd.index_type = primitive.index_type;
            cmd.index_offset = primitive.index_byte_offset;
            cmd.shader = shader;
            cmd.textures.push_back(shadow_bind);
            cmd.transform = world_model;
            cmd.attach_lights = true;
            cmd.cast_shadows = _cast_shadows;

            // Default to object-level material when no per-primitive glTF material is present.
            shader->set_uniform_int("u_use_diffuse_texture", 0);
            shader->set_uniform_vec4("u_diffuse_color", material.diffuse_color);

            if (primitive.material_index >= 0 && primitive.material_index < static_cast<int>(gpu_model.materials.size())) {
                const auto& mat = gpu_model.materials[primitive.material_index];
                shader->set_uniform_vec4("u_diffuse_color", mat.base_color_factor);

                if (mat.base_color_texture != 0) {
                    cmd.textures.push_back({ mat.base_color_texture, GL_TEXTURE_2D, 8, "u_diffuse_texture" });
                    shader->set_uniform_int("u_use_diffuse_texture", 1);
                }
            }

            renderer.submit(RenderPass::Forward, cmd);
        }
    }
}

void Object::set_model(Res::Model res) {
    r_model = res;
}

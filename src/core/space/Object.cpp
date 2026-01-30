#include "Object.hpp"
#include "core/rendering/Texture.hpp"
#include <iostream>

Object::Object(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale,
           const std::string& shader_path, 
           const std::string& model_path, 
           const std::string& texture_path) 
           : position(pos), rotation(rot), scale(scale) {
            _id = UUID<Object>{};
            r_shader.asset_path = shader_path;
            r_model.asset_path = model_path;
            r_texture.asset_path = texture_path;
           }

Object::~Object() {}

void Object::init(ResourceManager& resources, const std::string& name) {
    _name = name.empty() ? "O_" + std::to_string(_id.value()) : name;

    if (r_model.asset_path.empty()) {
        r_model = resources.load_model("res://models/VoxelModels/defaultCube.obj"); // default to cube if no model path is supplied
    } else {
        r_model = resources.load_model(r_model.asset_path);
    }

    auto shader_path = r_shader.asset_path;
    r_shader = resources.load_shader(shader_path, shader_path.substr(0, shader_path.find_last_of('.'))+".fs");

    // not used for now 
    // if (r_texture.asset_path != "") { 
    //     _texture = new Texture(resources.get_full_path(r_texture.asset_path));
    // } else {
    //     _texture = nullptr;
    // }
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

void Object::enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos, glm::vec3 sun_dir, glm::vec3 sun_color) {
    if (auto shader = resources.get_shader(r_shader.id)) {
        glm::mat4 proj = renderer.get_proj();
        glm::mat4 view = renderer.get_view();
        glm::mat4 model = get_model_matrix();
        (*shader)->hot_reload_if_changed();
        (*shader)->bind();
        (*shader)->set_uniform_vec3("u_camera_pos", camera_pos);
        (*shader)->set_uniform_vec3("u_sun_dir", sun_dir);
        (*shader)->set_uniform_vec3("u_sun_color", sun_color);
        (*shader)->set_uniform_mat4("u_mvp", proj * view * model);
        (*shader)->set_uniform_mat4("u_proj", renderer.get_proj());
        (*shader)->set_uniform_mat4("u_model", model);
        (*shader)->set_uniform_mat4("u_view", view);

        ModelGpuData gpu_model = resources.get_model_gpu_data(r_model.id);

        // STILL GOTTA HANDLE TEXTURE STUFF BUT IDC RIGHT NOW, WE MOVING FAST AND SWIFT, OUT WITH OLD, IN WITH NEW
        RenderCommand cmd{};
        cmd.vao = gpu_model.vao;
        cmd.draw_type = DrawType::Elements;
        cmd.count = gpu_model.index_count;
        cmd.shader = (*shader);
        cmd.attach_lights = true;

        renderer.submit(RenderPass::Forward, cmd);
    } else {
        std::cout << "Shader for resource ID: " << r_shader.id.value() << " not found!" << "\n";
        return;
    }
}

void Object::set_model(Res::Model res) {
    r_model = res;
}
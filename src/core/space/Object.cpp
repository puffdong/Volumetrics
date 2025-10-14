#include "Object.hpp"
#include "core/space/Space.hpp"
#include <iostream>

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

void Object::init(ResourceManager& resources, Space* space) {
    Base::init(resources, space);
    _model = new ModelObject(resources.get_full_path(r_model.asset_path));
    if (r_texture.asset_path != "") {
        _texture = new Texture(resources.get_full_path(r_texture.asset_path));
    } else {
        _texture = nullptr;
    }
    auto shader_path = r_shader.asset_path;
    r_shader = resources.load_shader(shader_path);
}

void Object::tick(float delta) {

}

void Object::enqueue(Renderer& renderer, ResourceManager& resources) {
    if (auto shader = resources.get_shader(r_shader.id)) {
        glm::mat4 proj = renderer.get_proj();
        glm::mat4 view = renderer.get_view();
        glm::mat4 model = get_model_matrix();
        (*shader)->hot_reload_if_changed();
        (*shader)->bind();
        (*shader)->SetUniform3f("u_camera_pos", _space->get_camera()->get_position());
        (*shader)->SetUniform3f("u_sun_dir", _space->get_sun()->get_direction());
        (*shader)->SetUniform3f("u_sun_color", glm::vec3(1.0, 1.0, 1.0));
        // (*shader)->SetUniform3f("", _space->get_sun()->get_color())
        (*shader)->SetUniformMat4("u_mvp", proj * view * model);
        (*shader)->SetUniformMat4("u_model_matrix", model);
        (*shader)->SetUniformMat4("u_view_matrix", view);

        // STILL GOTTA HANDLE TEXTURE STUFF BUT IDC RIGHT NOW, WE MOVING FAST AND SWIFT, OUT WITH OLD, IN WITH NEW
        RenderCommand cmd{};
        cmd.vao = _model->getVAO();;
        cmd.draw_type = DrawType::Elements;
        cmd.count          = _model->getIndexCount();
        cmd.model          = model;
        cmd.shader         = (*shader);

        renderer.submit(RenderPass::Forward, cmd);
    } else {
        std::cout << "Shader for resource ID: " << r_shader.id.value() << " not found!" << "\n";
        return;
    }
}

void Object::swap_model(ModelObject* model) {
    if (_model) {
        delete _model;
    } 
    _model = model;
}
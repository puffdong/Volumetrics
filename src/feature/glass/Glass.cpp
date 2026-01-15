#include "Glass.hpp"
#include <iostream>

void Glass::init(ResourceManager& resources) {
    r_shader = resources.load_shader("res://shaders/glass/glass.vs", "res://shaders/glass/glass.fs");

    position = glm::vec2(300.0f, 300.0f);
}

void Glass::tick(float delta, const ButtonMap& button_map) {
    if (button_map.MouseLeft && button_map.MousePointerActive) {
        position.x = button_map.MousePosX;
        position.y = button_map.MousePosY;
    }
}

void Glass::enqueue(Renderer& renderer, ResourceManager& resources, const ButtonMap& button_map) {
    if (!_visible) return;
    
    if (auto o_shader = resources.get_shader(r_shader.id)) {
        Shader* shader = (*o_shader);
        shader->hot_reload_if_changed();
        shader->bind();
        shader->set_uniform_float("u_far", renderer.get_far());
        shader->set_uniform_float("u_near", renderer.get_near());
        shader->set_uniform_vec2("u_resolution", renderer.get_viewport_size());

        glm::vec3 mouse;
        if (button_map.MousePointerActive && button_map.MouseLeft) {
            mouse = glm::vec3(button_map.MousePosX, button_map.MousePosY, 1.0);
            glm::vec3 hmm = glm::vec3(button_map.MousePosX / renderer.get_viewport_size().x, button_map.MousePosY / renderer.get_viewport_size().y, 1.0);
        } else {
            mouse = glm::vec3(button_map.MousePosX, button_map.MousePosY, 0.0);
        }
        
        shader->set_uniform_vec3("u_mouse_pos", mouse);
        shader->set_uniform_vec2("u_glass_pane_position", position);
        shader->set_uniform_float("u_glass_radius", 15.0); // 5 pixels
        shader->set_uniform_int("u_src_color", 0); // get it access to the u_src_color framebuffer texture!
        shader->set_uniform_int("u_depth_texture", 2);

        RenderCommand cmd{};
        cmd.draw_type = DrawType::FullscreenQuad;
        cmd.shader = shader;
        cmd.state.depth_write = false;

        renderer.submit(RenderPass::UI, cmd);

    } else {
        std::cout << "glass not liquid" << std::endl;
    }
}

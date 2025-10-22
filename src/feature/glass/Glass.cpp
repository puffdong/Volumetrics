#include "Glass.hpp"
#include "core/space/Space.hpp"
#include <iostream>

Glass::Glass() : Base(), glass_panes() {

}

void Glass::init(ResourceManager& resources, Space* space) {
    Base::init(resources, space);
    r_shader = resources.load_shader("res://shaders/glass/glass.vs", "res://shaders/glass/glass.fs");

    glass_panes.push_back({glm::vec2(300.0f, 300.0f)});
}

void Glass::tick(float delta) {
    const ButtonMap& button_map = _space->get_button_map();
    
    if (button_map.MouseLeft && button_map.MousePointerActive) {
        glass_panes[0].position.x = button_map.MousePosX;
        glass_panes[0].position.y = button_map.MousePosY;
    }
}

void Glass::enqueue(Renderer& renderer, ResourceManager& resources) {
    if (!_visible) return;

    if (auto o_shader = resources.get_shader(r_shader.id)) {
        Shader* shader = (*o_shader);
        
        shader->hot_reload_if_changed();
        shader->bind();
        shader->set_uniform_ivec2("u_resolution", renderer.get_framebuffer_size(RenderPass::UI));
        shader->set_uniform_ivec2("u_mouse_pos", glass_panes[0].position);


        RenderCommand cmd{};
        cmd.draw_type = DrawType::Framebuffer;
        cmd.shader    = shader;
        cmd.state.depth_write = false;

        renderer.submit(RenderPass::Volumetrics, cmd);

    } else {
        std::cout << "glass not liquid" << std::endl;
    }
}

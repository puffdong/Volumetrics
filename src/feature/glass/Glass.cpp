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
        const ButtonMap& bm = _space->get_button_map();
        
        Shader* shader = (*o_shader);
        shader->hot_reload_if_changed();
        shader->bind();
        shader->set_uniform_float("u_far", renderer.get_far());
        shader->set_uniform_float("u_near", renderer.get_near());
        shader->set_uniform_vec2("u_resolution", renderer.get_framebuffer_size(RenderPass::UI));
        
        // std::cout << renderer.get_framebuffer_size(RenderPass::UI).x << " <- x   y -> " << renderer.get_framebuffer_size(RenderPass::UI).y << std::endl;

        glm::vec3 mouse;
        if (bm.MousePointerActive && bm.MouseLeft) {
            mouse = glm::vec3(bm.MousePosX, bm.MousePosY, 1.0);
            // std::cout << "yes -> " << mouse.x << ", " << mouse.y << ", " << mouse.z << std::endl;
        } else {
            mouse = glm::vec3(bm.MousePosX, bm.MousePosY, 0.0);
        }
        

        shader->set_uniform_vec3("u_mouse_pos", mouse);
        shader->set_uniform_vec2("u_glass_pane_position", glass_panes[0].position);
        shader->set_uniform_float("u_glass_radius", 15.0); // 5 pixels
        shader->set_uniform_int("u_src_color", 0); // get it access to the u_src_color framebuffer texture!
        shader->set_uniform_int("u_depth_texture", 0); // get it access to the u_src_color framebuffer texture!

        RenderCommand cmd{};
        cmd.draw_type = DrawType::FullscreenQuad;
        cmd.shader    = shader;
        cmd.state.depth_write = false;

        renderer.submit(RenderPass::UI, cmd);

    } else {
        std::cout << "glass not liquid" << std::endl;
    }
}

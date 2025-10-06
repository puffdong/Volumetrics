#include "WaterSurface.hpp"
#include "core/rendering/Renderer.hpp"
#include <iostream>

WaterSurface::WaterSurface(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, Base* parent, float height, float width) 
: height(height), width(width), Base(pos, rot, scale, parent)
{
    
}

void WaterSurface::init(ResourceManager& resources, Space* space) {
    Base::init(resources, _space);
    r_shader = resources.load_shader("res://shaders/WaterSurface.shader");
    model = new ModelObject(height, width, 20, 20);
}

void WaterSurface::tick(float delta) {
    time += delta;
}

void WaterSurface::enqueue(Renderer& renderer, ResourceManager& resources) {
    glm::mat4 model_matrix = glm::scale(glm::translate(glm::mat4(1.f), position), glm::vec3(10.f, 10.f, 10.f));
    glm::mat4 mvp = renderer.get_proj() * renderer.get_view() * model_matrix;
    if (auto shader = resources.get_shader(r_shader.id)) {
        (*shader)->bind();
        (*shader)->SetUniform1i("u_Texture", 8);
        RenderCommand cmd{};
        cmd.vao        = model->getVAO();
        cmd.draw_type   = DrawType::Elements;
        cmd.count      = model->getIndexCount();
        cmd.model      = mvp; // swap this tbh, doesn't even make any semantic sense, the mvp is different from modelmtrx
        cmd.shader     = (*shader);
        cmd.state.depth_test   = true;
        cmd.state.depth_write  = true;
        cmd.state.cull_face    = true;
        cmd.state.line_smooth  = true;

        renderer.submit(RenderPass::Forward, cmd);
    } else {
        std::cout << "wateh" << std::endl;
    }
}
#include "WaterSurface.hpp"
#include "core/rendering/Renderer.hpp"
#include <iostream>

WaterSurface::WaterSurface(glm::vec3 position, float height, float width) 
: pos(position), height(height), width(width), time(0.f)
{
    shader = new Shader("/Users/puff/Developer/graphics/Volumetrics/res/shaders/WaterSurface.shader");
    model = new ModelObject(height, width, 20, 20);
}

void WaterSurface::tick(ButtonMap bm, float delta) {
    time += delta;
}

void WaterSurface::render(Renderer& renderer, glm::mat4 proj, glm::mat4 view, glm::vec3 camera_pos) {
    glm::mat4 modelTrans = glm::scale(glm::translate(glm::mat4(1.f), pos), glm::vec3(10.f, 10.f, 10.f));
	glm::mat4 mvp = proj * view * modelTrans;
    
    shader->Bind();
    GLCall(shader->SetUniform1i("u_Texture", 8));



    RenderCommand cmd{};
    cmd.vao        = model->getVAO();
    cmd.draw_type   = DrawType::Elements;
    cmd.count      = model->getIndexCount();
	cmd.model      = mvp; // swap this tbh, doesn't even make any semantic sense, the mvp is different from modelmtrx
    cmd.shader     = shader;
    cmd.state.depth_test   = true;
    cmd.state.depth_write  = true;
    cmd.state.cull_face    = true;
    cmd.state.line_smooth  = true;

    renderer.submit(RenderPass::Forward, cmd);

}
#include "WaterSurface.hpp"
#include "../../core/rendering/Renderer.h"
#include <iostream>

WaterSurface::WaterSurface(glm::vec3 position, float height, float width) 
: pos(position), height(height), width(width), time(0.f)
{
    shader = new Shader("/Users/puff/Developer/graphics/Volumetrics/res/shaders/WaterSurface.shader");
    model = new ModelObject(height, width, 20, 20); // float width, float depth, int numRows, int numCols
}

void WaterSurface::tick(ButtonMap bm, float delta) {
    time += delta;
}

void WaterSurface::render(glm::mat4 proj, glm::mat4 view, glm::vec3 camera_pos) {
    glm::mat4 modelTrans = glm::scale(glm::translate(glm::mat4(1.f), pos), glm::vec3(10.f, 10.f, 10.f));
	glm::mat4 mvp = proj * view * modelTrans;
    
    shader->Bind();
	// texture->Bind(0);
	shader->SetUniformMat4("u_MVP", mvp);
    GLCall(shader->SetUniform1i("u_Texture", 8));
    GLCall(model->render());

}
#include "Sun.hpp"
#include "../core/rendering/Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // For glm::lookAt (though not used directly in final method)
#include <glm/geometric.hpp>          // For glm::normalize, glm::cross, glm::length, glm::dot


Sun::Sun(glm::vec3 direction, glm::vec4 color) 
    : dir(direction), color(color), time(0.0f) {
        shader = new Shader("/Users/puff/Developer/graphics/Volumetrics/res/shaders/sun.shader");
        init_billboard_model();
    }

void Sun::tick(float delta) {
    time += delta;
    dir.x += 0.01 * sin(time); 
    dir.z += 0.01 * cos(time);
}

void Sun::init_billboard_model() {
    float depth = 5.0f;
    float width = 5.0f;
    std::vector<float> vertices = {
    // Positions                            // Normals            // Texture Coords
    -width / 2.0f, 0.0f, -depth / 2.0f,     0.0f, 1.0f, 0.0f,     0.0f, 0.0f,
     width / 2.0f, 0.0f, -depth / 2.0f,     0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
     width / 2.0f, 0.0f,  depth / 2.0f,     0.0f, 1.0f, 0.0f,     1.0f, 1.0f,
    -width / 2.0f, 0.0f,  depth / 2.0f,     0.0f, 1.0f, 0.0f,     0.0f, 1.0f
    };

    // Indices 
    std::vector<unsigned int> indices = { // this "pointed down" before so I swapped the order hah
        2, 1, 0,
        0, 3, 2
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    GLsizei stride = 8 * sizeof(float); // 3 + 3 + 2
    glEnableVertexAttribArray(0); // loc 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1); // loc 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(2); // loc 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));
    
    glBindVertexArray(0);

    index_count = static_cast<GLsizei>(indices.size());
}

void Sun::render(glm::mat4 proj, Camera* camera) {
    // glDisable(GL_DEPTH_TEST);
	// glDepthMask(GL_FALSE);

    // shitty shitty start ///////
    glm::vec3 cam_pos = camera->get_position();

    glm::vec3 norm_sun_dir = glm::normalize(this->dir); 
    glm::vec3 sun_pos = cam_pos + (norm_sun_dir * sun_distance);

    glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 new_Y_axis_ws = cam_pos - sun_pos;
    float distance_to_camera = glm::length(new_Y_axis_ws);
    new_Y_axis_ws /= glm::length(new_Y_axis_ws); 
    glm::vec3 new_X_axis_ws = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), new_Y_axis_ws));

    glm::vec3 new_Z_axis_ws = glm::normalize(glm::cross(new_X_axis_ws, new_Y_axis_ws));
    glm::mat4 rot(1.0f);
    rot[0] = glm::vec4(new_X_axis_ws, 0.0f);    
    rot[1] = glm::vec4(new_Y_axis_ws, 0.0f);    
    rot[2] = glm::vec4(new_Z_axis_ws, 0.0f);    

    glm::mat4 trans = glm::translate(glm::mat4(1.0f), sun_pos);
    glm::mat4 model_matrix = trans * rot;
    // shitty shitty end ///////

    glm::mat4 mvp = proj * camera->get_view_matrix() * model_matrix;

    shader->Bind();
    shader->SetUniform3f("sun_dir", this->dir); 
    // // shader->SetUniformMat4("mvp", mvp);

    // glBindVertexArray(VAO);
    // glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
    // glBindVertexArray(0);

    // glEnable(GL_DEPTH_TEST);
	// glDepthMask(GL_TRUE);

    RenderCommand cmd{};
    cmd.vao        = VAO;
    cmd.draw_type   = DrawType::Elements;
    cmd.count      = index_count;
	cmd.model      = mvp; // swap this tbh, doesn't even make any semantic sense, the mvp is different from modelmtrx
    cmd.shader     = shader;
	cmd.state.depth_test  = false;
    cmd.state.depth_write = false;

    Renderer::Submit(RenderPass::Skypass, cmd);

}
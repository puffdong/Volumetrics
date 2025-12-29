#include "Sun.hpp"
#include "core/space/Space.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <iostream>


Sun::Sun(glm::vec3 direction, glm::vec4 color) 
    : dir(direction), color(color), time(0.0f), Base() {
    }

void Sun::init(ResourceManager& resources, Space* space) {
    Base::init(resources, space);
    r_shader = resources.load_shader("res://shaders/sun.vs", "res://shaders/sun.fs");
    init_billboard_model();
}

void Sun::tick(float delta) {
    time += delta;
    dir.x += 0.005 * sin(time); 
    dir.z += 0.005 * cos(time);
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

    std::vector<unsigned int> indices = {
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

void Sun::enqueue(Renderer& renderer, ResourceManager& resources) {
    glm::vec3 cam_pos = _space->get_camera()->get_position();

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

    glm::mat4 mvp = renderer.get_proj() * renderer.get_view() * model_matrix;
    if (auto shader = resources.get_shader(r_shader.id)) {
        (*shader)->bind();
        (*shader)->set_uniform_vec3("sun_dir", this->dir); 
        (*shader)->set_uniform_mat4("u_mvp", mvp);

        RenderCommand cmd{};
        cmd.vao        = VAO;
        cmd.draw_type   = DrawType::Elements;
        cmd.count      = index_count;
        cmd.shader     = (*shader);
        cmd.state.depth_test  = false;
        cmd.state.depth_write = false;

        renderer.submit(RenderPass::Skypass, cmd);
    } else {
        std::cout << "sun ain't sunning" << std::endl;
    }
}
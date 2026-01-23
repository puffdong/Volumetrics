#include "Sun.hpp"
#include "core/rendering/Renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

Sun::Sun() = default;

Sun::~Sun() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ebo) glDeleteBuffers(1, &ebo);
}

void Sun::init(ResourceManager& resources) {
    r_shader = resources.load_shader("res://shaders/sun.vs", "res://shaders/sun.fs");
    init_billboard_model();
}

void Sun::tick(float delta) {
    float y = hmm; 
    
    if (_moving) {
        _time += delta * _speed;
    }

    float x = std::sin(_time);
    float z = std::cos(_time);

    direction = glm::vec3(x, y, z);
    direction = glm::normalize(direction);
}

void Sun::init_billboard_model() {
    const float depth = 15.0f;
    const float width = 15.0f;
    
    std::vector<float> vertices = {
    // Positions                            // Normals            // UVs
    -width / 2.0f, 0.0f, -depth / 2.0f,     0.0f, 1.0f, 0.0f,     0.0f, 0.0f,
     width / 2.0f, 0.0f, -depth / 2.0f,     0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
     width / 2.0f, 0.0f,  depth / 2.0f,     0.0f, 1.0f, 0.0f,     1.0f, 1.0f,
    -width / 2.0f, 0.0f,  depth / 2.0f,     0.0f, 1.0f, 0.0f,     0.0f, 1.0f
    };

    std::vector<unsigned int> indices = { 2, 1, 0, 0, 3, 2 };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    GLsizei stride = 8 * sizeof(float); 
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); 
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    
    glBindVertexArray(0);

    index_count = static_cast<GLsizei>(indices.size());
}

void Sun::enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos) {
    auto shader_ptr = resources.get_shader(r_shader.id);
    if (!shader_ptr) return;
    
    auto* shader = *shader_ptr;
    
    glm::vec3 sun_pos = camera_pos + (glm::normalize(direction) * sun_distance);

    glm::mat4 look_at = glm::lookAt(sun_pos, camera_pos, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model = glm::inverse(look_at);

    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 mvp = renderer.get_proj() * renderer.get_view() * model;

    shader->bind();
    shader->hot_reload_if_changed();
    shader->set_uniform_vec4("u_sun_color", color);
    shader->set_uniform_mat4("u_mvp", mvp);

    RenderCommand cmd{};
    cmd.vao = vao;
    cmd.draw_type = DrawType::Elements;
    cmd.count = index_count;
    cmd.shader = shader;
    
    cmd.state.depth_test = false;
    cmd.state.depth_write = false;

    renderer.submit(RenderPass::Skypass, cmd);
}
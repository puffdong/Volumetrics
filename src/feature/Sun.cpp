#include "Sun.hpp"
#include "core/rendering/Renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>

Sun::Sun() {
    set_direction(direction);
}

Sun::~Sun() {
    if (shader) {
        delete shader;
        shader = nullptr;
    }
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ebo) glDeleteBuffers(1, &ebo);
}

void Sun::init(ResourceManager& resources) {
    shader = new Shader(resources.get_full_path("res://shaders/sun.vs"), resources.get_full_path("res://shaders/sun.fs"));
    init_billboard_model();
}

void Sun::tick(float delta) {
    if (moving) {
        yaw += speed * delta;
        direction_dirty = true;
    }

    if (!direction_dirty) {
        return;
    }
    update_direction_from_angles();
}

void Sun::set_direction(const glm::vec3& dir) {
    const float len = glm::length(dir);
    direction = (len > 0.0001f) ? (dir / len) : glm::vec3(0.0f, 1.0f, 0.0f);

    pitch = glm::degrees(std::asin(glm::clamp(direction.y, -1.0f, 1.0f))); // update the pitch
    yaw = glm::degrees(std::atan2(direction.z, direction.x)); // and yaw when setting a new direction
    direction_dirty = false;
}

void Sun::set_pitch(float value) {
    pitch = glm::clamp(value, -89.9f, 89.9f);
    direction_dirty = true;
}

void Sun::set_yaw(float value) {
    yaw = value;
    direction_dirty = true;
}

void Sun::set_angles(float pitch_value, float yaw_value) {
    pitch = glm::clamp(pitch_value, -89.9f, 89.9f);
    yaw = yaw_value;
    direction_dirty = true;
}

void Sun::update_direction_from_angles() {
    const float pitch_rad = glm::radians(pitch);
    const float yaw_rad = glm::radians(yaw);

    glm::vec3 new_direction;
    new_direction.x = std::cos(pitch_rad) * std::cos(yaw_rad);
    new_direction.y = std::sin(pitch_rad);
    new_direction.z = std::cos(pitch_rad) * std::sin(yaw_rad);

    const float len = glm::length(new_direction);
    direction = (len > 0.0001f) ? (new_direction / len) : glm::vec3(0.0f, 1.0f, 0.0f);
    direction_dirty = false;
}

void Sun::init_billboard_model() {
    const float depth = 15.0f;
    const float width = 15.0f;
    
    std::vector<float> vertices = {
    // Positions                            // UVs
    -width / 2.0f, 0.0f, -depth / 2.0f,     0.0f, 0.0f,
     width / 2.0f, 0.0f, -depth / 2.0f,     1.0f, 0.0f,
     width / 2.0f, 0.0f,  depth / 2.0f,     1.0f, 1.0f,
    -width / 2.0f, 0.0f,  depth / 2.0f,     0.0f, 1.0f
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

    GLsizei stride = 5 * sizeof(float); 
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); 
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    
    glBindVertexArray(0);

    index_count = static_cast<GLsizei>(indices.size());
}

void Sun::enqueue(Renderer& renderer, glm::vec3 camera_pos) {
    if (!shader) return;
    
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
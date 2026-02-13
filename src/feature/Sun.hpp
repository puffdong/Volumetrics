#pragma once
#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"
#include "core/rendering/Shader.hpp"
#include <glm/glm.hpp>

class Sun {
public:
    glm::vec4 color{1.0f}; // .w = intensity
    float sun_distance = 250.0f;

    Sun();
    ~Sun();

    void init(ResourceManager& resources);
    void tick(float delta);
    void enqueue(Renderer& renderer, glm::vec3 camera_pos);

    glm::vec3 get_direction() const { return direction; }
    float get_pitch() const { return pitch; }
    float get_yaw() const { return yaw; }
    bool is_moving() const { return moving; }
    float get_speed() const { return speed; }
    glm::vec4 get_color() const { return color; }
    float get_intensity() const { return color.w; }
    float get_distance() const { return sun_distance; }
    
    void set_direction(const glm::vec3& dir);
    void set_pitch(float value);
    void set_yaw(float value);
    void set_angles(float pitch_value, float yaw_value);
    void set_moving(bool value) { moving = value; }
    void set_speed(float value) { speed = value; }
    void set_color(const glm::vec4& c) { color = c; }
    void set_intensity(const float i) { color.w = i; }
    void set_distance(const float dist) { sun_distance = dist; }

private:
    glm::vec3 direction{0.0f, 1.0f, 0.0f};
    float pitch = 0.0f;
    float yaw = 0.0f;
    bool moving = false;
    float speed = 30.0f;
    bool direction_dirty = true;

    Shader* shader = nullptr;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei index_count = 0;

    void init_billboard_model();
    void update_direction_from_angles();
};
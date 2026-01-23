#pragma once
#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"
#include <glm/glm.hpp>

class Sun {
public:
    glm::vec3 direction{0.0f, 1.0f, 0.0f};
    glm::vec4 color{1.0f};
    float sun_distance = 250.0f;
    float _time = 0.0f;
    bool _moving = true;
    float _speed = 0.05f;

    float hmm = 1.0f;

    Sun();
    ~Sun();

    void init(ResourceManager& resources);
    void tick(float delta);
    void enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos);

    glm::vec3 get_direction() const { return direction; }
    glm::vec4 get_color() const { return color; }
    float get_distance() const { return sun_distance; }
    bool get_moving() const { return _moving; }
    float get_speed() const { return _speed; }
    float get_hmm() const { return hmm; }
    
    void set_direction(const glm::vec3& dir) { direction = dir; }
    void set_color(const glm::vec4& c) { color = c; }
    void set_distance(const float dist) { sun_distance = dist; }
    void set_moving(const bool moving) { _moving = moving; }
    void set_speed(const float speed) { _speed = speed; }
    void set_hmm(const float val) { hmm = val; }

private:
    Resource r_shader;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei index_count = 0;

    void init_billboard_model();
};
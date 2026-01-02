#pragma once
#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"
#include <glm/glm.hpp>

class Sun {
public:
    glm::vec3 direction{0.0f, 1.0f, 0.0f};
    glm::vec4 color{1.0f};
    float sun_distance = 250.0f;
    float time = 0.0f;

    Sun();
    ~Sun();

    void init(ResourceManager& resources);
    void tick(float delta);
    void enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos);

    glm::vec3 get_direction() const { return direction; }
    glm::vec4 get_color() const { return color; }
    float get_distance() const { return sun_distance; }
    
    void set_direction(const glm::vec3& dir) { direction = dir; }
    void set_color(const glm::vec4& c) { color = c; }
    void set_distance(float dist) { sun_distance = dist; }

private:
    Resource r_shader;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei index_count = 0;

    void init_billboard_model();
};
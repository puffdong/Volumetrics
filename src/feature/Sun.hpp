#pragma once
#include "core/Base.hpp"
#include "glm/glm.hpp"

class Space; // fwd decl

class Sun : public Base {
public:
    Resource r_shader; 

	glm::vec3 dir;
    glm::vec4 color;

    float sun_distance = 250.f;

    float time;

	Sun(glm::vec3 direction, glm::vec4 color);

    void init(ResourceManager& resources, Space* space) override;
    void tick(float delta) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;
    
	glm::vec3 get_direction() { return dir; };
    glm::vec4 get_color() { return color; };
    float get_distance() { return sun_distance; };
    void set_distance(float distance) { sun_distance = distance; };


private:
    GLuint VAO, VBO, EBO;
    GLsizei index_count;
    

    void init_billboard_model();
};
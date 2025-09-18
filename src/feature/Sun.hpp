#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "core/rendering/Renderer.hpp"
#include "core/Camera.hpp"
#include "core/rendering/Shader.hpp"

class Sun {
public:
    Shader* shader;

	glm::vec3 dir;
    glm::vec4 color;

    float sun_distance = 250.f;

    float time;

	Sun(glm::vec3 direction, glm::vec4 color);

    void tick(float delta);
    void render(Renderer& renderer, Camera* camera);
	glm::vec3 get_direction() { return dir; }
    glm::vec4 get_color() { return color; }

private:
    GLuint VAO, VBO, EBO;
    GLsizei index_count;
    

    void init_billboard_model();
};
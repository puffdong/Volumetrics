#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"



class Sun {
public:
	glm::vec3 dir;
    glm::vec4 color;

	Sun(glm::vec3 direction, glm::vec4 color);

    void upload_sun_data();

	glm::vec3 get_direction() { return dir; }
    glm::vec4 get_color() { return color; }

private:
    GLuint ubo;

    void init_upload_buffer();
};
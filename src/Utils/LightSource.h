#pragma once
#include "glm/glm.hpp"

struct LightSource {
    glm::vec3 color;
    glm::vec3 dir;
    bool isDirectional;
    LightSource(glm::vec3 color, glm::vec3 dir, bool isDirectional) {
        this->color = color;
        this->dir = dir;
        this->isDirectional = isDirectional;
    }
};
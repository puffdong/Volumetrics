#pragma once

#include <glm/glm.hpp>
#include <cstddef>
#include "core/UUID.hpp"

enum class LightType : int {
    Point       = 0,
    Directional = 1
};

struct Light {
    glm::vec3 position   = glm::vec3(0.0f);
    float     radius     = 10.0f;

    glm::vec3 color      = glm::vec3(1.0f);
    float     intensity  = 1.0f;

    glm::vec3 direction  = glm::vec3(0.0f, 1.0f, 0.0f);
    float     volumetric_intensity = 1.0f;

    LightType type       = LightType::Point;
    UUID<Light> id       = UUID<Light>();
};

// the struct we convert to when uploading the light block! 
struct GpuLight {
    glm::vec4 position_radius;
    glm::vec4 color_intensity;
    glm::vec4 misc;
};

constexpr std::size_t MAX_LIGHTS = 16;

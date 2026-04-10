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
    float     angle      = 0.0f; // for spotlights, not implemented yet
    float     volumetric_multiplier = 1.0f;
    LightType type       = LightType::Point;
};

constexpr std::size_t MAX_LIGHTS = 16;

struct LightingData {
    glm::vec3 sun_direction;
    glm::vec3 sun_color;
    float sun_intensity;
    glm::vec3 ambient_color;
    float ambient_intensity;
};

struct GpuLight {
    glm::vec4 position_radius;
    glm::vec4 color_intensity;
    glm::vec4 direction_angle;
    glm::vec4 params; // x = type, y = volumetric multiplier, zw = padding
};

// the one block for all our lighting needs! :)
struct GpuLightBlock {
    glm::vec4 sun_direction; // .w = padding
    glm::vec4 sun_color_intensity; 
    glm::vec4 ambient_color_intensity;
    glm::ivec4 light_count_misc; // x = light count, yzw = padding
    
    GpuLight lights[MAX_LIGHTS];
};



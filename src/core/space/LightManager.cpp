#include "LightManager.hpp"
#include "core/rendering/Renderer.hpp"
#include <algorithm>
#include <iostream>

LightManager::~LightManager() {
    if (_ubo != 0) {
        glDeleteBuffers(1, &_ubo);
        _ubo = 0;
    }
}

void LightManager::init() {
    _capacity = MAX_LIGHTS;

    if (_ubo == 0) {
        glGenBuffers(1, &_ubo);
    }

    glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
    glBufferData(GL_UNIFORM_BUFFER,
                 sizeof(GpuLightBlock),
                 nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

std::vector<GpuLight> LightManager::convert_to_gpu_lights(const std::vector<Light>& lights) {
    std::vector<GpuLight> result(MAX_LIGHTS);

    for (std::size_t i = 0; i < lights.size() && i < MAX_LIGHTS; ++i) {
        const auto& l = lights[i];
        GpuLight g{};
        g.position_radius = glm::vec4(l.position, l.radius); // pos, radius
        g.color_intensity = glm::vec4(l.color, l.intensity); // color, intensity
        g.direction_angle = glm::vec4(l.direction, l.angle); // direction, angle
        g.params = glm::vec4(static_cast<float>(l.type), l.volumetric_multiplier, 0.0f, 0.0f); // type, volumetric multiplier, padding, padding
        result[i] = g;
    }

    return result;
}

void LightManager::upload(const LightingData& lighting_data, const std::vector<Light>& lights) {
    if (_capacity == 0) {
        return;
    }

    const int active_light_count = static_cast<int>(std::min(lights.size(), static_cast<std::size_t>(MAX_LIGHTS)));

    GpuLightBlock block{};
    block.sun_direction = glm::vec4(lighting_data.sun_direction, 1.0f);
    block.sun_color_intensity = glm::vec4(lighting_data.sun_color, lighting_data.sun_intensity);
    block.ambient_color_intensity = glm::vec4(lighting_data.ambient_color, lighting_data.ambient_intensity);
    block.light_count_misc = glm::ivec4(active_light_count, 0, 0, 0);
    
    std::vector<GpuLight> gpu_lights = convert_to_gpu_lights(lights);
    std::copy(gpu_lights.begin(), gpu_lights.end(), std::begin(block.lights));

    glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GpuLightBlock), &block);
    glBindBuffer(GL_UNIFORM_BUFFER, 0); // we are constantly re-uploading the same data every frame
    // would be nice to keep a cache if nothing happened. 
    // eh we will get to that at some point

    _current_count = active_light_count;
}

void LightManager::bind(unsigned int binding_point) const { // same thinking as a texture
    glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, _ubo);
}

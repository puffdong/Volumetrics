#include "LightManager.hpp"
#include "core/rendering/Renderer.hpp"
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
                 _capacity * sizeof(GpuLight),
                 nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

std::vector<GpuLight> LightManager::convert_to_gpu_lights(const std::vector<Light>& lights) {
    std::vector<GpuLight> result;
    result.reserve(MAX_LIGHTS);

    for (const auto& l : lights) {
        GpuLight g{};
        g.position_radius = glm::vec4(l.position, l.radius); // pos, radius
        g.color_intensity = glm::vec4(l.color, l.intensity); // color, intensity
        g.misc = glm::vec4(l.volumetric_intensity, static_cast<float>(l.type), 0.0f, 0.0f); // volumetric instensity, type, padding, padding
        result.push_back(g);
    }

    return result;
}

void LightManager::upload(const std::vector<Light>& lights) {
    if (_capacity == 0) {
        return;
    }

    std::vector<GpuLight> gpu_lights = LightManager::convert_to_gpu_lights(lights);

    glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, gpu_lights.size() * sizeof(GpuLight), gpu_lights.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0); // we are constantly re-uploading the same data every frame
    // would be nice to keep a cache if nothing happened. 
    // eh we will get to that at some point

    _current_count = static_cast<int>(gpu_lights.size());
}

void LightManager::bind(unsigned int binding_point) const { // same thinking as a texture
    glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, _ubo);
}

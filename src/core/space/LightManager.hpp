#pragma once
#include "Light.hpp"

#include <vector>
#include <cstddef>

class LightManager {
public:
    LightManager() = default;
    ~LightManager();

    void init();

    void bind(unsigned int binding_point) const;
    void upload(const std::vector<Light>& lights);
    
    std::vector<GpuLight> convert_to_gpu_lights(const std::vector<Light>& lights);

    int get_light_count() const { return _current_count; }


private:
    unsigned int _ubo = 0;
    std::size_t _capacity = 0;
    int _current_count = 0;
};

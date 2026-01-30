#pragma once

#include "imgui.h"
#include <vector>
#include "feature/raymarcher/raymarcher.hpp"
#include "feature/raymarcher/VoxelGrid.hpp"
#include "core/Camera.hpp"
#include "core/space/Object.hpp"
#include "feature/Sun.hpp"
#include "feature/glass/Glass.hpp"
#include "glm/glm.hpp"
#include "core/space/Light.hpp"

#define PI 3.14159265358979323846f

class Space; // fwd decl

namespace ui {
    void stats_overlay(Camera& camera, Renderer& renderer);
    inline float to_deg(float r) { return r * (180.0f / 3.1415926535f); }
    inline float to_rad(float d) { return d * (3.1415926535f / 180.0f); } 

    void object_settings(std::vector<Object*>& objects);
    
    void raymarch_settings(Raymarcher& marcher, RaymarchSettings& ray_settings);
    
    void voxel_grid_settings(VoxelGrid& grid); // includes corner visualization toggle
    
    void settings_panel(Space& space, Raymarcher& marcher, RaymarchSettings& ray_settings, VoxelGrid& grid, Sun& sun, std::vector<Light>& lights, Glass& glass, std::vector<Object*>& objects);

    void light_settings(Space& space, Sun& sun, std::vector<Light>& lights);

    void glass_settings(Glass& glass);
}

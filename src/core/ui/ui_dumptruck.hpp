#pragma once

// I just need a quick and dirty way to facilitate imgui stuff for debug purposes
// so, heed my dumptruck, it is dirty, it is quick, and I am probably going to
// regret this later on. future self will love my past self

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

    void transform_window(Object& obj, const char* title);

    void raymarch_settings(Raymarcher& marcher, RaymarchSettings& ray_settings);

    void voxel_grid_settings(VoxelGrid& grid); // includes corner visualization toggle

    void settings_panel(Space& space, Raymarcher& marcher, RaymarchSettings& ray_settings, VoxelGrid& grid, Sun& sun, std::vector<Light>& lights, Glass& glass);

    void light_settings(Space& space, Sun& sun, std::vector<Light>& lights);

    void glass_settings(Glass& glass);
}

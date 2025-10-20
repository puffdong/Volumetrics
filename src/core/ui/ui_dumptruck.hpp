#pragma once

// I just need a quick and dirty way to facilitate imgui stuff for debug purposes
// so, heed my dumptruck, it is dirty, it is quick, and I am probably going to
// regret this later on. future self will love my past self

#include "imgui.h"
#include "feature/raymarcher/raymarcher.hpp"
#include "feature/raymarcher/VoxelGrid.hpp"
#include "core/Camera.hpp"
#include "core/Base.hpp"
#include "glm/glm.hpp"

#define PI 3.14159265358979323846f

namespace ui {
    void stats_overlay(Camera* camera);
    inline float to_deg(float r) { return r * (180.0f / 3.1415926535f); }
    inline float to_rad(float d) { return d * (3.1415926535f / 180.0f); } 

    void transform_window(Base& obj, const char* title);

    void raymarch_settings(Raymarcher& marcher, RaymarchSettings& ray_settings);

    void voxel_grid_settings(VoxelGrid& grid);

    void raymarcher_panel(Raymarcher& marcher, RaymarchSettings& ray_settings, VoxelGrid& grid);
}

#include "ui_dumptruck.hpp"
#include "imgui.h"
#include "feature/raymarcher/raymarcher.hpp"
#include "feature/raymarcher/VoxelGrid.hpp"
#include "core/Camera.hpp"
#include "core/space/Object.hpp"
#include "glm/glm.hpp"
#include <iostream>

#define PI 3.14159265358979323846f

namespace ui {
    
    void stats_overlay(Camera& camera, Renderer& renderer) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                 ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiCond_Always);
        if (ImGui::Begin("##overlay", nullptr, flags)) {
            ImGui::Text("FOV: %.1f FPS: %.1f", renderer.get_fov(), ImGui::GetIO().Framerate);
            glm::vec2 viewport_size = renderer.get_viewport_size();
            ImGui::Text("Resolution: %d x %d", (int)viewport_size.x, (int)viewport_size.y);
            ImGui::Separator();
            ImGui::Text("pos: %.1f, %.1f, %.1f",
                camera.position.x,
                camera.position.y,
                camera.position.z);
            ImGui::Separator();
            ImGui::Text("dir: %.1f, %.1f, %.1f",
                camera.front.x,
                camera.front.y,
                camera.front.z);
            
        }
        ImGui::End();
    }

    void transform_window(Object& obj, const char* title)
    {
        ImGui::PushID(obj.get_id());

        if (ImGui::Begin(title)) {
            // Position (xyz)
            glm::vec3 p = obj.get_position();
            float pos[3] = { p.x, p.y, p.z };
            if (ImGui::InputFloat3("Position", pos, "%.3f")) {
                obj.set_position({ pos[0], pos[1], pos[2] });
            }

            glm::vec3 r = obj.get_rotation();
            float rot[3] = { r.x, r.y, r.z };
            if (ImGui::SliderFloat3("Rotation", rot, -PI, PI, "%.3f",
                                    ImGuiSliderFlags_AlwaysClamp)) {
                obj.set_rotation({ rot[0], rot[1], rot[2] });
            }
            
            glm::vec3 scale = obj.get_scale();
            float s[3] = {scale.x, scale.y, scale.z};
            if (ImGui::SliderFloat3("Scale", s, 0.01f, 10.0f, "%.3f",
                                   ImGuiSliderFlags_AlwaysClamp)) {
                obj.set_scale({ s[0], s[1], s[2] });
            }
        }
        ImGui::End();
        ImGui::PopID();
    }

    void raymarch_settings(Raymarcher& marcher, RaymarchSettings& ray_settings) {
        {
            bool visible = marcher.is_visible();
            if (ImGui::Checkbox("Toggle Raymarching (on/off)", &visible)) {
                marcher.set_visibility(visible);
            }
        }

        // Reset all to struct defaults
        if (ImGui::SmallButton("Reset to defaults##raymarch_settings")) {
            ray_settings = RaymarchSettings{}; // leverage default member initializers
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Reset all sliders to RaymarchSettings defaults");
        ImGui::Separator();

        
        // --- Marching ---
        ImGui::SliderInt("Max steps", &ray_settings.max_steps, 1, 1024);
        ImGui::SliderFloat("Step size", &ray_settings.step_size, 0.001f, 3.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Hit step size (unused)", &ray_settings.hit_step_size, 0.00001f, 1.0f, "%.5f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Max distance", &ray_settings.max_distance, 0.01f, 4096.0f, "%.2f",
                        ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Min distance (epsilon)", &ray_settings.min_distance, 1e-6f, 1e-1f, "%.6f",
                        ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);

        ImGui::Separator();

        // --- Lighting / Shadow March ---
        ImGui::SliderInt("Max light steps", &ray_settings.max_light_steps, 0, 256);
        ImGui::SliderFloat("Light step size", &ray_settings.light_step_size, 0.001f, 5.0f, "%.4f", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Separator();

        // --- Medium Coefficients ---
        ImGui::SliderFloat("Absorption coefficient", &ray_settings.absorption_coefficient, 0.0f, 5.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Scattering coefficient", &ray_settings.scattering_coefficient, 0.0f, 5.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Extinction coefficient", &ray_settings.extincion_coefficient, 0.0f, 5.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        
        ImGui::Separator();

        ImGui::SliderFloat("Anisotropy", &ray_settings.anisotropy, 0.01f, 3.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Sun intensity", &ray_settings.sun_intensity, 1.0f, 250.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Text("Base color (RGB)");
        ImGui::SliderFloat("R##base_color", &ray_settings.base_color.x, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("G##base_color", &ray_settings.base_color.y, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("B##base_color", &ray_settings.base_color.z, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    }

    void voxel_grid_settings(VoxelGrid& grid) {
        {
            bool visible = grid.is_debug_view_visible();
            if (ImGui::Checkbox("Show voxel grid debug view", &visible)) {
                grid.set_debug_visibility(visible);
            }
        }
        // Cell size slider (clamped 0.1f..50.0f)
        {
            float cell_size = grid.get_cell_size();
            if (ImGui::SliderFloat("Cell size", &cell_size, 0.1f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
                grid.set_cell_size(cell_size);
            }
        }

        // Position sliders (X, Y, Z)
        {
            glm::vec3 p = grid.get_position();
            float x = p.x, y = p.y, z = p.z;

            bool changed = false;
            changed |= ImGui::SliderFloat("X##grid_pos", &x, -50.0f, 50.0f, "%.3f");
            changed |= ImGui::SliderFloat("Y##grid_pos", &y, -50.0f, 50.0f, "%.3f");
            changed |= ImGui::SliderFloat("Z##grid_pos", &z, -50.0f, 50.0f, "%.3f");

            if (changed) {
                grid.set_position(glm::vec3{x, y, z});
            }
        }
    }


    void settings_panel(Raymarcher& marcher, RaymarchSettings& ray_settings, VoxelGrid& grid, Sun& sun, std::vector<Light>& lights, Glass& glass)
    {
        ImGui::PushID(&marcher);

        if (ImGui::Begin("Rendering Settings")) {
            if (ImGui::CollapsingHeader("Light Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                light_settings(sun, lights);
            }
            if (ImGui::CollapsingHeader("Raymarch settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                raymarch_settings(marcher, ray_settings);
            }
            if (ImGui::CollapsingHeader("Voxel grid settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                voxel_grid_settings(grid);
            }
            if (ImGui::CollapsingHeader("Glass settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                glass_settings(glass);
            }

        }

        ImGui::End();
        ImGui::PopID();
    }
    
    void light_settings(Sun& sun, std::vector<Light>& lights) {
        bool moving = sun.get_moving();
        if (ImGui::Checkbox("Moving", &moving)) {
            sun.set_moving(moving);
        }
        float speed = sun.get_speed();
        if (moving && ImGui::SliderFloat("Speed", &speed, 0.01f, 3.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
            sun.set_speed(speed);
        }
        float hmm = sun.get_hmm();
        if (ImGui::SliderFloat("Hmm", &hmm, -3.0f, 3.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
            sun.set_hmm(hmm);
        }
        ImGui::Separator();

        glm::vec4 color = sun.get_color();
        // sliders to change the rgb values of sun color
        ImGui::SliderFloat("R##sun_color", &color.r, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("G##sun_color", &color.g, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("B##sun_color", &color.b, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        sun.set_color(color);

        ImGui::Separator();

        Light& light1 = lights[0];
        ImGui::Text("Light 1 Settings:");
        // position
        ImGui::SliderFloat("X##light1_pos", &light1.position.x, -50.0f, 50.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Y##light1_pos", &light1.position.y, -50.0f, 50.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Z##light1_pos", &light1.position.z, -50.0f, 50.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        // radius
        ImGui::SliderFloat("Radius##light1_radius", &light1.radius, 0.1f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        // color
        ImGui::SliderFloat("R##light1_color", &light1.color.r, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("G##light1_color", &light1.color.g, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("B##light1_color", &light1.color.b, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        // intensity
        ImGui::SliderFloat("Intensity##light1_intensity", &light1.intensity, 0.0f, 500.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);


        ImGui::Separator();

        Light& light2 = lights[1];
        ImGui::Text("Light 2 Settings:");
        // position
        ImGui::SliderFloat("X##light2_pos", &light2.position.x, -50.0f, 50.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Y##light2_pos", &light2.position.y, -50.0f, 50.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Z##light2_pos", &light2.position.z, -50.0f, 50.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        // radius
        ImGui::SliderFloat("Radius##light2_radius", &light2.radius, 0.1f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        // color
        ImGui::SliderFloat("R##light2_color", &light2.color.r, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("G##light2_color", &light2.color.g, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("B##light2_color", &light2.color.b, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        // intensity
        ImGui::SliderFloat("Intensity##light2_intensity", &light2.intensity, 0.0f, 500.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Separator();
        
        Light& light3 = lights[2];
        ImGui::Text("Light 3 Settings:");
        // position
        ImGui::SliderFloat("X##light3_pos", &light3.position.x, -50.0f, 50.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Y##light3_pos", &light3.position.y, -50.0f, 50.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Z##light3_pos", &light3.position.z, -50.0f, 50.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        // radius
        ImGui::SliderFloat("Radius##light3_radius", &light3.radius, 0.1f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        // color
        ImGui::SliderFloat("R##light3_color", &light3.color.r, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("G##light3_color", &light3.color.g, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("B##light3_color", &light3.color.b, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        // intensity
        ImGui::SliderFloat("Intensity##light3_intensity", &light3.intensity, 0.0f, 500.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);


    }

    void glass_settings(Glass& glass) {
        bool visible = glass.is_visible();
        if (ImGui::Checkbox("Toggle Glass", &visible)) {
            glass.set_visibility(visible);
        }
    }
}

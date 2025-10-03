// I just need a quick and dirty way to facilitate imgui stuff for debug purposes
// so, heed my dumptruck, it is dirty, it is quick, and I am probably going to
// regret this later on. future self will love my past self
#pragma /* frickin */ once

#include "imgui.h"
#include "core/space/Space.hpp"
#include "core/Camera.hpp"
#include "core/Base.hpp"
#include "glm/glm.hpp"

#define PI 3.14159265358979323846f

namespace ui {
    
    static void stats_overlay(Camera* camera) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                 ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiCond_Always);
        if (ImGui::Begin("##overlay", nullptr, flags)) {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Separator();
            ImGui::Text("c_pos: %.1f, %.1f, %.1f",
                camera->position.x,
                camera->position.y,
                camera->position.z);
            ImGui::Separator();
            ImGui::Text("c_dir: %.1f, %.1f, %.1f",
                camera->front.x,
                camera->front.y,
                camera->front.z);
        }
        ImGui::End();
    }

    static inline float to_deg(float r) { return r * (180.0f / 3.1415926535f); }
    static inline float to_rad(float d) { return d * (3.1415926535f / 180.0f); } 

    static void transform_window(Base& obj, const char* title)
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

    static void raymarcher_panel(Raymarcher& marcher, VoxelGrid& grid)
    {
        ImGui::PushID(&grid);

        if (ImGui::Begin("Raymarcher")) {

            if (ImGui::CollapsingHeader("Voxel Grid", ImGuiTreeNodeFlags_DefaultOpen)) {

                {
                    glm::vec3 p = grid.get_position();
                    float pos[3] = { p.x, p.y, p.z };
                    if (ImGui::InputFloat3("Grid position", pos, "%.3f")) {
                        grid.set_position(glm::vec3{ pos[0], pos[1], pos[2] });
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Reset##grid_pos")) {
                        grid.set_position(glm::vec3{0.0f, 0.0f, 0.0f});
                    }
                }

                {
                    float cell_size = grid.get_cell_size();
                    if (ImGui::DragFloat("Cell size", &cell_size, 0.01f, 0.0001f, 1000.0f, "%.4f",
                                         ImGuiSliderFlags_AlwaysClamp)) {
                        if (cell_size < 0.0001f) cell_size = 0.0001f;
                        grid.set_cell_size(cell_size);
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Reset##cell_size")) {
                        grid.set_cell_size(1.0f);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("voxel size bruddah");
                }
            }

            if (ImGui::CollapsingHeader("Marcher", 0)) {
                ImGui::TextDisabled("soon brother, soon");
            }
        }

        ImGui::End();
        ImGui::PopID();
    }
}

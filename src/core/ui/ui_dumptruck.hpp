// I just need a quick and dirty way to facilitate imgui stuff for debug purposes
// so, heed my dumptruck, it is dirty, it is quick, and I am probably going to
// regret this later on. future self will love my past self
#pragma /* frickin */ once

#include "imgui.h"
#include "../Space.h"
#include "../Camera.h"
#include "../WorldObject.h"
#include "../glm/glm.hpp"

#define PI 3.14159265358979323846f

namespace ui {
    
    static void stats_overlay(Camera* camera) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                 ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        if (ImGui::Begin("##overlay", nullptr, flags)) {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Separator();
            ImGui::Text("Camera pos: %.2f, %.2f, %.2f",
                camera->position.x,
                camera->position.y,
                camera->position.z);
            ImGui::Separator();
            ImGui::Text("Camera dir: %.2f, %.2f, %.2f",
                camera->front.x,
                camera->front.y,
                camera->front.z);
        }
        ImGui::End();
    }

    static inline float to_deg(float r) { return r * (180.0f / 3.1415926535f); }
    static inline float to_rad(float d) { return d * (3.1415926535f / 180.0f); } 
    
    static void transform_window(WorldObject& obj, const char* title)
    {
        ImGui::PushID(&obj);

        if (ImGui::Begin(title)) {
            // Position (xyz)
            glm::vec3 p = obj.getPosition();
            float pos[3] = { p.x, p.y, p.z };
            if (ImGui::InputFloat3("Position", pos, "%.3f")) {
                obj.setPosition({ pos[0], pos[1], pos[2] });
            }

            glm::vec3 r = obj.getRotation();
            float rot[3] = { r.x, r.y, r.z };
            if (ImGui::SliderFloat3("Rotation", rot, -PI, PI, "%.3f",
                                    ImGuiSliderFlags_AlwaysClamp)) {
                obj.setRotation({ rot[0], rot[1], rot[2] });
            }

            glm::vec3 scale = obj.getScale();
            float s[3] = {scale.x, scale.y, scale.z};
            if (ImGui::SliderFloat3("Scale", s, 0.01f, 10.0f, "%.3f",
                                   ImGuiSliderFlags_AlwaysClamp)) {
                obj.setScale({ s[0], s[1], s[2] });
            }
        }
        ImGui::End();
        ImGui::PopID();
    }
}

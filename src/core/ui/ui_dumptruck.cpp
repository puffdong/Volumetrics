#include "ui_dumptruck.hpp"
#include "core/space/Space.hpp"
#include <iostream>

#define PI 3.14159265358979323846f

namespace ui {

    void stats_overlay(Camera& camera, Renderer& renderer) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                 ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::SetNextWindowPos(ImVec2(5, 24), ImGuiCond_Always);
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

    void object_settings(std::vector<Object*>& objects) {
        for (std::size_t i = 0; i < objects.size(); ++i) {
            Object* obj = objects[i];
            if (!obj) {
                continue;
            }
            ImGui::PushID(obj->get_id());

            std::string title = obj->get_name();
            bool open =ImGui::BeginMenu(title.c_str());
            bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
            obj->set_selected(hovered);

            if (open) {
                // Transform section
                ImGui::Text("Transform");
                ImGui::Separator();
                
                glm::vec3 p = obj->get_position();
                float pos[3] = { p.x, p.y, p.z };
                if (ImGui::DragFloat3("Position", pos, 0.1f, -1000.0f, 1000.0f, "%.3f")) {
                    obj->set_position({ pos[0], pos[1], pos[2] });
                }

                glm::vec3 r = obj->get_rotation();
                float rot[3] = { r.x, r.y, r.z };
                if (ImGui::SliderFloat3("Rotation", rot, -PI, PI, "%.3f",
                                        ImGuiSliderFlags_AlwaysClamp)) {
                    obj->set_rotation({ rot[0], rot[1], rot[2] });
                }

                glm::vec3 scale = obj->get_scale();
                float s[3] = { scale.x, scale.y, scale.z };
                if (ImGui::SliderFloat3("Scale", s, 0.01f, 10.0f, "%.3f",
                                       ImGuiSliderFlags_AlwaysClamp)) {
                    obj->set_scale({ s[0], s[1], s[2] });
                }

                ImGui::Separator();
                
                // Material section
                ImGui::Text("Material");
                ImGui::Separator();
                
                Material& mat = obj->get_material();
                
                ImGui::Text("Diffuse");
                ImGui::ColorEdit3("Color##diffuse", &mat.diffuse_color.r);
                ImGui::SliderFloat("Strength##diffuse", &mat.diffuse_color.a, 0.0f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                
                ImGui::Separator();
                
                ImGui::Text("Specular");
                ImGui::ColorEdit3("Color##specular", &mat.specular_color.r);
                ImGui::SliderFloat("Strength##specular", &mat.specular_color.a, 0.0f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                
                ImGui::Separator();
                
                ImGui::SliderFloat("Shininess", &mat.params.x, 1.0f, 256.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Higher = sharper specular highlights");
                
                // todo: do this! Your future you is gonna love it.
                // ImGui::SliderFloat("Metallic", &mat.params.y, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                // ImGui::SliderFloat("Roughness", &mat.params.z, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

                ImGui::EndMenu();
            }

            ImGui::PopID();
        }
    }

    void raymarch_settings(Raymarcher& marcher, RaymarchSettings& ray_settings) {
        {
            bool visible = marcher.is_visible();
            if (ImGui::Checkbox("Raymarch", &visible)) {
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

        ImGui::SliderInt("Max steps", &ray_settings.max_steps, 1, 512);
        ImGui::SliderFloat("Step size", &ray_settings.step_size, 0.001f, 3.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Separator();

        ImGui::SliderInt("Max light steps", &ray_settings.max_light_steps, 0, 256);
        ImGui::SliderFloat("Light step size", &ray_settings.light_step_size, 0.001f, 5.0f, "%.4f", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Separator();

        ImGui::SliderFloat("Absorption coefficient", &ray_settings.absorption_coefficient, 0.0f, 5.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Scattering coefficient", &ray_settings.scattering_coefficient, 0.0f, 5.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        
        ImGui::Separator();

        ImGui::SliderFloat("Anisotropy", &ray_settings.anisotropy, 0.01f, 3.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Sun intensity multiplier", &ray_settings.sun_intensity_multiplier, 1.0f, 250.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Text("Base color (RGB)");
        ImGui::SliderFloat("R##base_color", &ray_settings.base_color.x, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("G##base_color", &ray_settings.base_color.y, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("B##base_color", &ray_settings.base_color.z, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    }

    void perlin_noise_settings(PerlinNoiseTexture& texture) {
        ImGui::Text("Perlin Noise Texture Settings");
        ImGui::Text("Dimensions: %d x %d x %d", texture.width, texture.height, texture.depth);
        ImGui::Text("Frequency: %.3f", texture.frequency);
        ImGui::Text("Current Seed: %u", texture.seed);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Click to copy");
        }
        if (ImGui::IsItemClicked()) {
            ImGui::SetClipboardText(std::to_string(texture.seed).c_str());
        }
        ImGui::Separator();
        
        static unsigned int new_seed = 0;
        
        ImGui::InputScalar("Seed", ImGuiDataType_U32, &new_seed);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("0 = random seed");
        
        static float new_frequency = 0.05f;
        ImGui::DragFloat("Frequency", &new_frequency, 0.01f, 0.01f, 10.0f, "%.3f");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Controls the scale of the noise pattern");
        
        if (ImGui::Button("Regenerate Noise")) {
            re_init_perlin(texture, new_frequency, new_seed);
            generate_perlin(texture);
            upload_perlin(texture);
        }
    }

    void voxel_grid_settings(VoxelGrid& grid) {
        {
            bool visible = grid.is_debug_view_visible();
            if (ImGui::Checkbox("Show Voxel Grid", &visible)) {
                grid.set_debug_visibility(visible);
            }
        }
        {
            bool show_selection_box = grid.is_selection_box_enabled();
            if (ImGui::Checkbox("Show Selection Box", &show_selection_box)) {
                grid.set_selection_box_enabled(show_selection_box);
            }
            uint8_t selection_value = grid.get_selection_value();
            int display_value = static_cast<int>(selection_value);
            // ImGui::SameLine();
            if (ImGui::DragInt("Selection Value", &display_value, 1, 0, 255)) {
                grid.set_selection_value(static_cast<uint8_t>(display_value));
            }
        }
        // Cell size slider (clamped 0.01f..25.0f)
        {
            float cell_size = grid.get_cell_size();
            if (ImGui::SliderFloat("Cell size", &cell_size, 0.1f, 25.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
                grid.set_cell_size(cell_size);
            }
        }

        // Position sliders (X, Y, Z)
        {
            glm::vec3 p = grid.get_position();
            if (ImGui::DragFloat3("Position##grid_pos", &p.x, 0.1f, -1000.0f, 1000.0f, "%.3f")) {
                grid.set_position(p);
            }
        }
        ImGui::Separator();
        {
            glm::ivec3 cur = grid.get_grid_dim();
            ImGui::Text("Size (%d, %d, %d)", cur.x, cur.y, cur.z);

            static bool init_dims = false;
            static bool preserve_data = false;
            static int dims[3] = { 1, 1, 1 };
            if (!init_dims) {
                dims[0] = cur.x; dims[1] = cur.y; dims[2] = cur.z;
                init_dims = true;
            }
            
            if (ImGui::InputInt3("", dims)) {
                if (dims[0] < 1) dims[0] = 1;
                if (dims[1] < 1) dims[1] = 1;
                if (dims[2] < 1) dims[2] = 1;
            }
            ImGui::SameLine();
            if (ImGui::Button("Resize")) {
                grid.resize_grid(dims[0], dims[1], dims[2], preserve_data);
            }
            ImGui::SameLine();
            ImGui::Checkbox("Preserve", &preserve_data);
        }
    }


    void settings_panel(Space& space, Raymarcher& marcher, RaymarchSettings& ray_settings, VoxelGrid& grid, Sun& sun, std::vector<Light>& lights, Glass& glass, Line& line_manager, std::vector<Object*>& objects)
    {
        ImGui::PushID(&marcher);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Objects")) {
                object_settings(objects);
                ImGui::EndMenu();
            }            
            if (ImGui::BeginMenu("Lights")) {
                light_settings(space, sun, lights);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Raymarcher")) {
                raymarch_settings(marcher, ray_settings);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Perlin Noise")) {
                perlin_noise_settings(marcher.get_perlin_texture());
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Voxel Grid")) {
                voxel_grid_settings(grid);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Glass")) {
                glass_settings(glass);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Other")) {
                if (ImGui::BeginMenu("Lines")) {
                    line_settings(line_manager);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGui::PopID();
    }
    
    void light_settings(Space& space, Sun& sun, std::vector<Light>& lights) {
        ImGui::Text("Sun");
        bool moving = sun.is_moving();
        if (ImGui::Checkbox("Moving", &moving)) {
            sun.set_moving(moving);
        }

        float speed = sun.get_speed();
        if (ImGui::SliderFloat("Speed", &speed, 0.0f, 180.0f, "%.2f deg/s", ImGuiSliderFlags_AlwaysClamp)) {
            sun.set_speed(speed);
        }
        ImGui::Separator();

        float pitch = sun.get_pitch();
        float yaw = sun.get_yaw();
        bool changed = false;
        changed |= ImGui::SliderFloat("Pitch", &pitch, -89.9f, 89.9f, "%.1f deg", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::SliderFloat("Yaw", &yaw, -180.0f, 180.0f, "%.1f deg", ImGuiSliderFlags_AlwaysClamp);
        if (changed) {
            sun.set_angles(pitch, yaw);
        }
        ImGui::Separator();

        glm::vec4 color = sun.get_color();
        ImGui::DragFloat3("Color##sun_color", &color.r, 0.01f, 0.0f, 1.0f, "%.2f");
        sun.set_color(color);
        ImGui::Separator();
        float intensity = sun.get_intensity();
        if (ImGui::DragFloat("Intensity##sun_intensity", &intensity, 0.01f, 0.0f, 1000.0f, "%.2f")) {
            sun.set_intensity(intensity);
        }

        ImGui::Separator();

        int remove_index = -1;
        for (std::size_t i = 0; i < lights.size(); ++i) {
            Light& light = lights[i];
            ImGui::PushID(static_cast<int>(i));
            std::string label = "Light " + std::to_string(i + 1);
            if (ImGui::BeginMenu(label.c_str())) {
                ImGui::Text("%s Settings:", label.c_str());
                ImGui::DragFloat3("Position##light_pos", &light.position.x, 0.1f, -1000.0f, 1000.0f, "%.2f");
                ImGui::SliderFloat("Radius##light_radius", &light.radius, 0.1f, 200.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::SliderFloat("R##light_color", &light.color.r, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::SliderFloat("G##light_color", &light.color.g, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::SliderFloat("B##light_color", &light.color.b, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::SliderFloat("Intensity##light_intensity", &light.intensity, 0.0f, 1000.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::SliderFloat("Volumetric Multiplier##light_vol_multiplier", &light.volumetric_multiplier, 0.0f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

                ImGui::Separator();
                if (ImGui::Button("Remove Light")) {
                    remove_index = static_cast<int>(i);
                }
                ImGui::EndMenu();
            }
            ImGui::PopID();
        }
        if (remove_index >= 0) {
            space.remove_light(static_cast<std::size_t>(remove_index));
        }
        ImGui::Separator();
        if (ImGui::Button("Add Light")) {
            space.add_light(
                glm::vec3(0.0f, 5.0f, 0.0f), 50.0f,
                glm::vec3(1.0f), 50.0f, 
                glm::vec3(0.0f, -1.0f, 0.0f), 1.0f, LightType::Point
            );
        }
    }

    void line_settings(Line& line_manager) {
        if (ImGui::Button("Clear Lines")) {
            line_manager.clear_lines();
        }
        if (ImGui::Button("Clear All Lines")) {
            line_manager.clear_all_lines();
        }
    }

    void glass_settings(Glass& glass) {
        bool visible = glass.is_visible();
        if (ImGui::Checkbox("Toggle Glass", &visible)) {
            glass.set_visibility(visible);
        }
    }
}

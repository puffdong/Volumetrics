#pragma once
#include <iostream>
#include <string>
#include <stdexcept>

// OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// core
#include "core/rendering/Renderer.hpp"
#include "core/space/Space.hpp"
#include "core/resources/ResourceManager.hpp"
#include "core/ui/ui_dumptruck.hpp"
#include "core/utils/ButtonMap.hpp"

struct AppConfig {
    int initial_width;
    int initial_height;
    std::string assets_root_path;
};

class Application {    
public:
    explicit Application(const AppConfig& cfg);
    int run();
    
private:
    GLFWwindow* window;
    Space* space;
    Renderer renderer;
    ResourceManager resources;
    ButtonMap bm;

    float last_time;

    float fov = 70.f;
    
    bool init_glfw(const AppConfig& cfg);
    int shutdown();

    // callback functions and respective variables
    bool mouse_active = true;
    bool  first_mouse = true;
    float last_x = 800.0f;
    float last_y = 450.0f;
    void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    int pending_width = 0;
    int pending_height = 0;
    bool resize_dirty = true;
    double last_resize_event = 0.0;
    const double RESIZE_SETTLE = 0.10;
    void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
};
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

#include "./core/rendering/Renderer.h"
#include "./core/rendering/Shader.h"
#include "./core/rendering/Texture.h"


#include "core/Space.h"
#include "core/Camera.h"

// utils
#include "utils/ButtonMap.h"

ButtonMap bm;
Space* space;

static bool mouse_active = true;
static bool  firstMouse = true;
static float lastX = 800.0f;
static float lastY = 450.0f;

static int pendingW  = 0, pendingH = 0;
static bool resizeDirty = true;
static double lastResizeEvent = 0.0;
const  double RESIZE_SETTLE = 0.10;   // seconds

void mouse_callback(GLFWwindow*, double xpos, double ypos)
{
    if (mouse_active) {
        if (firstMouse) {
            lastX = (float)xpos;
            lastY = (float)ypos;
            firstMouse = false;
        }

        float xoffset = (float)xpos - lastX;
        float yoffset = lastY - (float)ypos;   // y‑axis is inverted
        lastX = (float)xpos;
        lastY = (float)ypos;

        space->get_camera()->process_mouse(xoffset, yoffset);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);   // hide cursor and take control of it
        mouse_active = true;
        firstMouse = true;
    }

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    space->change_fov(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mouse_active = false;
        }
        else {
            glfwSetWindowShouldClose(window, true);
        }
    }
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_W:
            bm.W = true;
            break;
        case GLFW_KEY_A:
            bm.A = true;
            break;
        case GLFW_KEY_S:
            bm.S = true;
            break;
        case GLFW_KEY_D:
            bm.D = true;
            break;
        case GLFW_KEY_SPACE:
            bm.Space = true;
            break;
        case GLFW_KEY_UP:
            bm.Up = true;
            break;
        case GLFW_KEY_DOWN:
            bm.Down = true;
            break;
        case GLFW_KEY_LEFT:
            bm.Left = true;
            break;
        case GLFW_KEY_RIGHT:
            bm.Right = true;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            bm.Ctrl = true;
            break;
        }
    }
    else if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_W:
            bm.W = false;
            break;
        case GLFW_KEY_A:
            bm.A = false;
            break;
        case GLFW_KEY_S:
            bm.S = false;
            break;
        case GLFW_KEY_D:
            bm.D = false;
            break;
        case GLFW_KEY_SPACE:
            bm.Space = false;
            break;
        case GLFW_KEY_UP:
            bm.Up = false;
            break;
        case GLFW_KEY_DOWN:
            bm.Down = false;
            break;
        case GLFW_KEY_LEFT:
            bm.Left = false;
            break;
        case GLFW_KEY_RIGHT:
            bm.Right = false;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            bm.Ctrl = false;
            break;
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);         // cheap, do this immediately
    pendingW = width;
    pendingH = height;
    resizeDirty = true;
    lastResizeEvent = glfwGetTime(); // record when the last event arrived
}


int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    window = glfwCreateWindow(1300, 800, "volumetrics", NULL, NULL);
    if (!window)
    {
        std::cout << "umm glfw didnt work" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // sync with refresh rate

    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);   // hide cursor and take control of it
    glfwSetCursorPosCallback(window, mouse_callback);              // set the cursor callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;

    // ImGui_ImplGlfw_InitForOpenGL(window, true);
    // ImGui_ImplOpenGL3_Init();

    if (glewInit() != GLEW_OK) {
        std::cout << "glew init Error!" << std::endl;
    }

    int initial_width, initial_height;
    glfwGetFramebufferSize(window, &initial_width, &initial_height);

    glViewport(0, 0, initial_width, initial_width);
    
    GLCall(glDepthFunc(GL_LESS));
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glEnable(GL_CULL_FACE));
    GLCall(glCullFace(GL_BACK));

    GLuint globalVao = 0;
    GLCall(glGenVertexArrays(1, &globalVao));
    GLCall(glBindVertexArray(globalVao));

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    Renderer::InitRenderer(initial_height, initial_width);
    glViewport(0,0, initial_width, initial_height);

    space = new Space();

    if (space) {
        space->update_projection_matrix_aspect_ratio(static_cast<float>(initial_width) / initial_height);
    }

    float lastTime = glfwGetTime();

    pendingH = initial_height;
    pendingW = initial_width;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (resizeDirty && glfwGetTime() - lastResizeEvent > RESIZE_SETTLE) {
            Renderer::Resize(pendingW, pendingH);
            if (space) space->update_projection_matrix_aspect_ratio(static_cast<float>(pendingW) / pendingH);
            resizeDirty = false;

            std::cout << "viewport resized to (" << pendingW << ", " << pendingH << ")" << std::endl;
        }

        // ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();
        // ImGui::ShowDemoWindow(); // Show demo window! :)
        Renderer::BeginFrame({0.1f, 0.1f, 0.2f, 1.0f});

        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        space->tick(deltaTime, bm);
        space->enqueue_renderables();
        Renderer::ExecutePipeline();
        Renderer::PresentToScreen(); // testing stuff

        space->renderWorld(deltaTime);

        // ImGui::Render();
        // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
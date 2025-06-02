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


#include "./rendering/Renderer.h"
#include "./rendering/Shader.h"
#include "./rendering/Texture.h"


#include "core/Space.h"
#include "core/Camera.h"

// utils
#include "utils/ButtonMap.h"
#include "utils/perlin_noise_generator.hpp"

ButtonMap bm;
Space* space;

static bool mouse_active = true;
static bool  firstMouse = true;
static float lastX = 800.0f;
static float lastY = 450.0f;

static int currentFbWidth_main = 1600; // Default
static int currentFbHeight_main = 900; // Default

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

// static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
// {
//     space->process_cursor_position(xpos, ypos);
// }

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Close the window when the user presses the ESC key
    // ESC = release mouse; second ESC = close window
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

// At the top of main.cpp with other globals, or pass Space* via glfwSetWindowUserPointer
// For simplicity, assuming 'space' is accessible globally here as it is in your current code.
// int currentFramebufferWidth = 1600; // Initial, will be updated
// int currentFramebufferHeight = 900;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Update glViewport to cover the new framebuffer size
    glViewport(0, 0, width, height);
    currentFbWidth_main = width;   // Update global static width
    currentFbHeight_main = height; // Update global static height
    if (space) { // Ensure the space object exists
        space->update_projection_matrix_aspect_ratio((float)width / (float)height);
    }
}

GLuint generatePerlin2D() {
    PerlinNoiseTexture perlinTexture2D(512, 512);
    GLuint textureID2D = perlinTexture2D.getTextureID();
    return textureID2D;
}

// GLuint generatePerlin3D() {
//     PerlinNoiseTexture perlinTexture3D(128, 128, 128);
//     GLuint textureID3D = perlinTexture3D.getTextureID();
//     return textureID3D;
// }

void save_perlin() {
    PerlinNoiseTexture perlinTexture2D(512, 512, "C:/Dev/OpenGL/Volumetrics/testing/test.ppm");
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

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1600, 900, "volumetrics", NULL, NULL);
    if (!window)
    {
        std::cout << "umm glfw didnt work" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // sync with refresh rate

    // std::cout << glGetString(GL_VERSION) << std::endl; // 4.1 INTEL-23.0.26

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch


    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);   // hide cursor and take control of it
    glfwSetCursorPosCallback(window, mouse_callback);              // set the cursor callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glfwSetCursorPosCallback(window, cursor_position_callback);

    glewExperimental = GL_TRUE;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    if (glewInit() != GLEW_OK) {
        std::cout << "glew init Error!" << std::endl;
    }
    
    int initialFbWidth, initialFbHeight;
    glfwGetFramebufferSize(window, &initialFbWidth, &initialFbHeight);

    glViewport(0, 0, initialFbWidth, initialFbHeight);
    // GLCall(glClearDepth(1.0f));
    glDepthFunc(GL_LESS);
    glClearColor(1.0f, 0.0f, 0.3f, 1.0f); // Set background to a dark gray


    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glEnable(GL_CULL_FACE));
    GLCall(glCullFace(GL_BACK));

    GLuint globalVao = 0;
    GLCall(glGenVertexArrays(1, &globalVao));
    GLCall(glBindVertexArray(globalVao));

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    space = new Space();

    if (space) {
        space->update_projection_matrix_aspect_ratio((float)initialFbWidth / (float)initialFbHeight);
    }

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(); // Show demo window! :)

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear yuh

        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        space->tick(deltaTime, bm);
        space->renderWorld(deltaTime);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
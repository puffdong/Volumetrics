#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>


#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"


#include "VoxelPlayground/Space.h"

#include "VoxelPlayground/Camera.h"

// Utils
#include "Utils/ButtonMap.h"
#include "Utils/perlin_noise_generator.hpp"

ButtonMap bm;
Space* space;

static bool mouse_active = true;
static bool  firstMouse = true;
static float lastX = 800.0f;
static float lastY = 450.0f;

void mouse_callback(GLFWwindow*, double xpos, double ypos)
{   
    if (mouse_active) {
        if (firstMouse) {
            lastX = (float) xpos;
            lastY = (float) ypos;
            firstMouse = false;
        }

        float xoffset = (float) xpos - lastX;
        float yoffset = lastY - (float) ypos;   // y‑axis is inverted
        lastX = (float) xpos;
        lastY = (float) ypos;

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
    window = glfwCreateWindow(1600, 900, "Volumetrics", NULL, NULL);
    if (!window)
    {
        std::cout << "umm glfw didnt work" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); // sync with refresh rate
    
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);   // hide cursor and take control of it
    glfwSetCursorPosCallback(window, mouse_callback);              // set the cursor callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // glfwSetCursorPosCallback(window, cursor_position_callback);
    
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        std::cout << "glew init Error!" << std::endl;
    }


    GLCall(glViewport(0, 0, 1600, 900));
    GLCall(glClearColor(0.1f, 0.1f, 0.8f, 1.0f)); // Set background to a dark gray


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLCall(glEnable(GL_DEPTH_TEST));
    glEnable(GL_CULL_FACE);

    Renderer renderer;

    Texture texture("C:/Dev/OpenGL/Volumetrics/res/textures/grass.tga");
    texture.Bind();
    texture.Unbind();

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // std::cout << "Generating perlin noise" << std::endl;

    // GLuint perlin2d = generatePerlin2D();
    // GLuint perlin3d = generatePerlin3D();
    // save_perlin();
    // std::cout << "Finished generating perlin noise" << std::endl;

    space = new Space();

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        renderer.Clear();

        space->tick(deltaTime, bm);
        space->renderWorld(deltaTime);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
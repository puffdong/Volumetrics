#include "Application.hpp"

Application::Application(const AppConfig& cfg) {
    init(cfg);
}

bool Application::Application::init(const AppConfig& cfg) {
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    window = glfwCreateWindow(1920, 1080, "volumetrics", NULL, NULL);
    if (!window)
    {
        std::cout << "umm glfw didnt work" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // sync with refresh rate

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // the callback logic is jank, but if it ain't broke don't fix... prolly gon regret this
    glfwSetWindowUserPointer(window, this); // to get glfwGetWindowUserPointer to work
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int sc, int action, int mods){
        if (auto* self = static_cast<Application*>(glfwGetWindowUserPointer(w)))
        self->key_callback(w, key, sc, action, mods);
    });
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y){
        if (auto* self = static_cast<Application*>(glfwGetWindowUserPointer(w)))
            self->mouse_callback(w, x, y);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int b, int a, int m){
        if (auto* self = static_cast<Application*>(glfwGetWindowUserPointer(w)))
            self->mouse_button_callback(w, b, a, m);
    });

    glfwSetScrollCallback(window, [](GLFWwindow* w, double dx, double dy){
        if (auto* self = static_cast<Application*>(glfwGetWindowUserPointer(w)))
            self->scroll_callback(w, dx, dy);
    });

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int W, int H){
        if (auto* self = static_cast<Application*>(glfwGetWindowUserPointer(w)))
            self->framebuffer_resize_callback(w, W, H);
    });

    glewExperimental = GL_TRUE;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    if (glewInit() != GLEW_OK) {
        std::cout << "glew init Error!" << std::endl;
    }

    int initial_width = cfg.initial_width;
    int initial_height = cfg.initial_height;
    glfwGetFramebufferSize(window, &initial_width, &initial_height);

    glViewport(0, 0, initial_width, initial_height);
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    Renderer::InitRenderer(initial_width, initial_height);

    space = new Space();

    if (space) {
        space->update_projection_matrix_aspect_ratio(static_cast<float>(initial_width) / initial_height);
    }

    float lastTime = glfwGetTime();

    pending_width = initial_width;
    pending_height = initial_height;
}

void Application::mouse_callback(GLFWwindow*, double xpos, double ypos)
{
    if (mouse_active) {
        if (first_mouse) {
            last_x = (float) xpos;
            last_y = (float) ypos;
            first_mouse = false;
        }

        float xoffset = (float) xpos - last_x;
        float yoffset = last_y - (float) ypos;   // y‑axis is inverted
        last_x = (float) xpos;
        last_y = (float) ypos;

        space->get_camera()->process_mouse(xoffset, yoffset);
    }
}

void Application::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);   // hide cursor and control it
        mouse_active = true;
        first_mouse = true;
    }

}

void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    space->change_fov(xoffset, yoffset);
}

void Application::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

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
            bm.LeftCtrl = true;
            break;
        case GLFW_KEY_LEFT_SHIFT: 
            bm.LeftShift = true;
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
            bm.LeftCtrl = false;
            break;
        case GLFW_KEY_LEFT_SHIFT: 
            bm.LeftShift = false;
            break;
        }
    }
}

void Application::framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    pending_width = width;
    pending_height = height;
    resize_dirty = true;
    last_resize_event = glfwGetTime();
}

int Application::run() {
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (resize_dirty && glfwGetTime() - last_resize_event > RESIZE_SETTLE) {
            Renderer::Resize(pending_width, pending_height);
            if (space) space->update_projection_matrix_aspect_ratio(static_cast<float>(pending_width) / pending_height);
            resize_dirty = false;

            std::cout << "viewport resized to (" << pending_width << ", " << pending_height << ")" << std::endl;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Renderer::BeginFrame({0.1f, 0.1f, 0.2f, 1.0f});

        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        space->tick(deltaTime, bm);
        space->enqueue_renderables();

        Renderer::ExecutePipeline();

        // ui 
        ui::stats_overlay(space->get_camera());

        // Imgui again
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
}
#include "Application.hpp"

Application::Application(const AppConfig& cfg) : resources(cfg.assets_root_path) {
    bool success = init_glfw(cfg);
    if (!success) {
        std::cout << "glfw failed to initialize" << std::endl;
    }
    int initial_width, initial_height; 
    if (cfg.platform == "Windows") { // bit jank... I don't understand what macos' resolution scaling is doing
        glfwGetFramebufferSize(window, &initial_width, &initial_height); // hypothesis: macos returns 2x framebuffer when on "retina display"
    } else {
        glfwGetWindowSize(window, &initial_width, &initial_height); // continued hypothesis: hack solution of getting window size yields the actual precieved size. 
    }   // my mac is too sluggish to handle all the stuff in this project, can't have it 2x itself into oblivion

    pending_width = initial_width; 
    pending_height = initial_height;

    renderer.init_renderer(initial_width, initial_height);
    float aspect_ratio = static_cast<float>(initial_width) / initial_height;
    renderer.set_projection_matrix(aspect_ratio, 70.f, 0.1f, 512.f);

    space = new Space(resources, renderer);

    last_time = (float) glfwGetTime();
}

bool Application::init_glfw(const AppConfig& cfg) {
    int initial_width = cfg.initial_width;
    int initial_height = cfg.initial_height;
    
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    if (cfg.platform == "macOS") {
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
    }


    window = glfwCreateWindow(initial_width, initial_height, "volumetrics", NULL, NULL);
    if (!window)
    {
        std::cout << "umm glfw didnt work" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // sync with refresh rate

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // disabled for now, its causing issues eh eh

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
        return false;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    return true;
}

void Application::mouse_callback(GLFWwindow*, double xpos, double ypos)
{
    if (camera_control_mouse_active) {
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

    mouse_pos_x = xpos;
    mouse_pos_y = ypos;
}

void Application::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if ((button == GLFW_MOUSE_BUTTON_RIGHT) && (action == GLFW_PRESS) && (camera_control_mouse_active == false)) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);   // hide cursor and control it
        camera_control_mouse_active = true;
        first_mouse = true;
    } else if ((button == GLFW_MOUSE_BUTTON_RIGHT) && (action == GLFW_PRESS) && (camera_control_mouse_active == true)) {
        space->cast_ray();
    }

    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            button_map.MouseRight = true;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            button_map.MouseLeft = true;
        }
    } else if (action == GLFW_RELEASE) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            button_map.MouseRight = false;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            button_map.MouseLeft = false;
        }
    }

}

void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (camera_control_mouse_active) {
        fov -= (float) yoffset; // we only want to set it when controlling the camera!
        renderer.set_fov(fov);
    }
}

void Application::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            camera_control_mouse_active = false;
        }
        else {
            glfwSetWindowShouldClose(window, true);
        }
    }
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_W:
            button_map.W = true;
            break;
        case GLFW_KEY_A:
            button_map.A = true;
            break;
        case GLFW_KEY_S:
            button_map.S = true;
            break;
        case GLFW_KEY_D:
            button_map.D = true;
            break;
        case GLFW_KEY_SPACE:
            button_map.Space = true;
            break;
        case GLFW_KEY_UP:
            button_map.Up = true;
            break;
        case GLFW_KEY_DOWN:
            button_map.Down = true;
            break;
        case GLFW_KEY_LEFT:
            button_map.Left = true;
            break;
        case GLFW_KEY_RIGHT:
            button_map.Right = true;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            button_map.LeftCtrl = true;
            break;
        case GLFW_KEY_LEFT_SHIFT: 
            button_map.LeftShift = true;
            break;

        }
    }
    else if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_W:
            button_map.W = false;
            break;
        case GLFW_KEY_A:
            button_map.A = false;
            break;
        case GLFW_KEY_S:
            button_map.S = false;
            break;
        case GLFW_KEY_D:
            button_map.D = false;
            break;
        case GLFW_KEY_SPACE:
            button_map.Space = false;
            break;
        case GLFW_KEY_UP:
            button_map.Up = false;
            break;
        case GLFW_KEY_DOWN:
            button_map.Down = false;
            break;
        case GLFW_KEY_LEFT:
            button_map.Left = false;
            break;
        case GLFW_KEY_RIGHT:
            button_map.Right = false;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            button_map.LeftCtrl = false;
            break;
        case GLFW_KEY_LEFT_SHIFT: 
            button_map.LeftShift = false;
            break;
        }
    }
}

void Application::framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); // This needs to be here, having it later causes graphical glitches for 1 frame.
    pending_width = width;
    pending_height = height;
    resize_dirty = true;
    last_resize_event = glfwGetTime();
}

int Application::run() {
    running = true;
    while (running)
    {
        glfwPollEvents();

        if (resize_dirty && glfwGetTime() - last_resize_event > RESIZE_SETTLE) {
            renderer.resize(pending_width, pending_height);
            renderer.set_projection_matrix(static_cast<float>(pending_width) / pending_height, fov);
            resize_dirty = false;

            std::cout << "viewport resized to (" << pending_width << ", " << pending_height << ")" << std::endl;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderer.begin_frame();

        button_map.MousePosX = mouse_pos_x; // needed a quick an dirty way to get mouse pointer pos through.
        button_map.MousePosY = mouse_pos_y;
        if (camera_control_mouse_active) {
            button_map.MousePointerActive = false;
        } else {
            button_map.MousePointerActive = true;
        }

        float current_time = glfwGetTime();
        float delta_time = current_time - last_time;
        last_time = current_time;
        space->tick(delta_time, button_map);
        space->enqueue_renderables();

        renderer.execute_pipeline();

        // ui 
        ui::stats_overlay(space->get_camera(), renderer);

        // Imgui again
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        if (glfwWindowShouldClose(window)) {
            stop();
        }
    }
    return shutdown();
}

void Application::stop() {
    running = false;
}

int Application::shutdown() {
    std::cout << "Shutting down..."; 
    delete space;

    renderer.destroy_renderer();
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
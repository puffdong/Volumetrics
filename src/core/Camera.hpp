
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/utils/ButtonMap.hpp"

class Camera
{
public:
    glm::vec3 position;
    glm::vec3 front; // Direction camera is facing
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float movement_speed;
    float rotation_speed;
    float mouse_sensitivity;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f,
        float pitch = 0.0f);

    glm::mat4 get_view_matrix();
    glm::vec3 get_position();
    glm::vec3 get_right();
    glm::vec3 get_front() const { return front; };
    void process_mouse(float xOffset, float yOffset, bool constrainPitch = true);

    void tick(float deltaTime, const ButtonMap& bm);

private:
    void process_keyboard(const ButtonMap& bm, float deltaTime);
    void process_rotation(const ButtonMap& bm, float deltaTime);
    void update_camera_vectors();
};

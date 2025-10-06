#include "Camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : position(position),
    worldUp(up),
    yaw(yaw),
    pitch(pitch),
    front(glm::vec3(0.0f, 0.0f, -1.0f)),
    movement_speed(5.0f),
    rotation_speed(50.0f),
    mouse_sensitivity(0.1f)
{
    update_camera_vectors();
}

void Camera::update_camera_vectors()
{
    // Calculate the new Front vector
    glm::vec3 frontVec;
    frontVec.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    frontVec.y = sin(glm::radians(pitch));
    frontVec.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(frontVec);
    // Re-calculate the Right and Up vectors
    right = glm::normalize(glm::cross(front, worldUp));  // Normalize the vectors
    up = glm::normalize(glm::cross(right, front));
}

glm::mat4 Camera::get_view_matrix()
{
    return glm::lookAt(position, position + front, up);
}

glm::vec3 Camera::get_position() {
    return position;
}

glm::vec3 Camera::get_right() {
    return right;
}

void Camera::process_keyboard(const ButtonMap& bm, float deltaTime)
{
    float velocity = movement_speed * deltaTime;
    if (bm.LeftShift) velocity *= 2.5f;
    if (bm.W)
        position += front * velocity;
    if (bm.S)
        position -= front * velocity;
    if (bm.A)
        position -= right * velocity;
    if (bm.D)
        position += right * velocity;
    if (bm.Space)
        position += worldUp * velocity;
    if (bm.LeftCtrl)
        position -= worldUp * velocity;
}

void Camera::process_rotation(const ButtonMap& bm, float deltaTime)
{
    float rotation = rotation_speed * deltaTime;
    if (bm.Left)
        yaw -= rotation;
    if (bm.Right)
        yaw += rotation;
    if (bm.Up)
    {
        pitch += rotation;
        if (pitch > 89.0f)
            pitch = 89.0f;
    }
    if (bm.Down)
    {
        pitch -= rotation;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }
    // Update Front, Right and Up Vectors using the updated Euler angles
    update_camera_vectors();
}


void Camera::process_mouse(float xOffset, float yOffset, bool constrainPitch)
{
    xOffset *= mouse_sensitivity;
    yOffset *= mouse_sensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if (constrainPitch)
    {
        if (pitch > 89.0f)  pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    update_camera_vectors();
}

void Camera::tick(float deltaTime, const ButtonMap& bm)
{
    process_keyboard(bm, deltaTime);
    process_rotation(bm, deltaTime);
}





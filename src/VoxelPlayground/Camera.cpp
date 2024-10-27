#include "Camera.h"

Camera::Camera(glm::vec3 front)
{
	dir = glm::normalize(front);
	up_vector = glm::vec3(0.f, 1.f, 0.f);
	pitch = 0.f;
	current_distance = kTargetDistance;
	yaw = acos(glm::dot(dir, up_vector) / (glm::length(up_vector) * glm::length(dir)));
	position = glm::vec3(0.0f);
}

void Camera::updateTargetPos(glm::vec3 new_target)
{
	target = new_target; // player->getPosition();
}

glm::mat4 Camera::getLookAt()
{

	glm::vec3 dir(
		cos(yaw) * cos(pitch),
		sin(pitch),
		sin(yaw) * cos(pitch)
	);
	dir = glm::normalize(dir);

	return glm::lookAt(getPosition(), target, up_vector);
}

glm::vec3 Camera::getPosition()
{
	return target - dir * current_distance;
}

void Camera::tick(float delta, ButtonMap bm)
{
	

	int pitchDir = 0;
	int yawDir = 0;

	if (bm.Up)
		pitchDir -= 1;
	if (bm.Down)
		pitchDir += 1;
	if (bm.Left)
		yawDir += 1;
	if (bm.Right)
		yawDir -= 1;

	if (pitchDir != 0 || yawDir != 0) {
		rotate(
			(float)pitchDir * kSensitivity * delta,
			(float)yawDir * kSensitivity * delta
		);
	}

	glm::vec3 movement = glm::vec3(0.0);

	if (bm.W) {
		movement += glm::vec3(
			sin(yaw),
			0.0,
			cos(yaw));
	}
		
	if (bm.S) {
		movement -= glm::vec3(
			sin(yaw),
			0.0,
			cos(yaw));
		}
		
	if (bm.Space)
		movement += glm::vec3(0.f, 1.0, 0.0);

	if (bm.Ctrl)
		movement += glm::vec3(0.f, -1.0, 0.0);

	position += movement * 5.f * delta;

	updateTargetPos(position);
}

void Camera::rotate(float pitchDiff, float yawDiff)
{
	pitch += pitchDiff * kSensitivity;
	yaw += yawDiff * kSensitivity;
	const float pi = 3.14159265358979323846;
	float angleLimit = (float)(pi / 2.f * 0.99f);
	if (pitch > angleLimit)
		pitch = angleLimit;
	else if (pitch < -angleLimit)
		pitch = -angleLimit;

	glm::vec3 tmpdir(0.f);
	// tmpdir.x = cos(mYaw - pPlayerYaw) * cos(mPitch);
	// tmpdir.y = sin(mPitch);
	// tmpdir.z = sin(mYaw - pPlayerYaw) * cos(mPitch);
	tmpdir.x = cos(yaw) * cos(pitch);
	tmpdir.y = sin(pitch);
	tmpdir.z = sin(yaw) * cos(pitch);
	dir = glm::normalize(tmpdir);
}

std::string Camera::toString()
{
	return "Target position: " + glm::to_string(target) + ", Direction: " + glm::to_string(dir) + ", Pitch: " + std::to_string(pitch) + ", Yaw: " + std::to_string(yaw);
}